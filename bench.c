#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>


#define MAX_HOST_LENGTH 256
#define DEFAULT_THREADS 1
#define DEFAULT_MAX_THREADS 128
#define DEFAULT_REQUESTS_PER_THREAD 10
#define PORT "80"


/* This structure hold data for each thread */
typedef struct {
    int thread_id;                          // Unique identifier for each thread
    double total_latency;                   // Sum of latencies for each request made by this thread
    char host[MAX_HOST_LENGTH];             // Hostname for the HTTP request
    int requests_per_thread;                // Number of HTTP requests per thread
} ThreadData;

/* Performs HTTP requests */
void* performRequest(void* arg) {
    ThreadData* data = (ThreadData*)arg;    // Cast 'arg' to 'ThreadData*' to use it as ThreadData

    /* Structures for storing address information */
    struct addrinfo hints, *res;
    int sockfd;                             // Socket descriptor for network communication
    char request[1024];
    snprintf(request, sizeof(request), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", data->host);
    char buffer[4096];                      // Buffer to store response
    struct timespec start, end;             // Variables to measure time for latency calculation
    double latency;                         // Variable to store the latency of each request
    
    /* Initialize hints to zero and set it up for IPv4/IPv6 and TCP */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;            // Either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;        // TCP socket

    /* Resolve the server's address using DNS */
    if (getaddrinfo(data->host, PORT, &hints, &res) != 0) {
        perror("getaddrinfo failed");       // Print an error message if DNS resolution fails
        return NULL;
    }

    /* Loop to make the specified number of HTTP requests */
    for (int i = 0; i < data->requests_per_thread; ++i) {
        /* Create a TCP socket */
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0) {
            perror("Error creating socket");
            continue;
        }

        /* Connect the socket to the server using the resolved address */
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("Error connecting to server");
            close(sockfd);                  // Close the socket if connection fails
            continue;
        }

        /* Record the start time before sending the request */
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        /* Send the HTTP GET request */
        if (send(sockfd, request, strlen(request), 0) < 0) {
            perror("Error sending request");
            close(sockfd);                  // Close the socket if send fails
            continue;
        }

        /* Receive the response from the server */
        if (recv(sockfd, buffer, sizeof(buffer) - 1, 0) < 0) {
            perror("Error receiving response");
            close(sockfd);                  // Close the socket if receive fails
            continue;
        }

        
        /* Record the end time after receiving the response */
        clock_gettime(CLOCK_MONOTONIC, &end);    
        /* Calculate the latency for this request */
        latency = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        data->total_latency += latency;     // Add the latency to the total for this thread

        close(sockfd);                      // Close the socket after processing the request
    }

    freeaddrinfo(res);                      // Free the memory allocated for the resolved address
    return NULL;
}

int main(int argc, char *argv[]) {
    int max_threads = DEFAULT_MAX_THREADS;
    int requests_per_thread = DEFAULT_REQUESTS_PER_THREAD;
    char *host;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <host> [<requests_per_thread>]\n", argv[0]);
        return 1;
    }

    host = argv[1];

    if (argc >= 3) {
        requests_per_thread = atoi(argv[2]);
        if (requests_per_thread <= 0) {
            requests_per_thread = DEFAULT_REQUESTS_PER_THREAD;
        }
    }

    /* Determine the number of CPU cores to create appropriate number of threads */
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores <= 0) {
        num_cores = DEFAULT_THREADS;
    }
    if (max_threads > num_cores) {
        max_threads = num_cores;
    }

    printf("Executing test with %d threads, each making %d requests to host: %s\n", max_threads, requests_per_thread, host);

    pthread_t threads[max_threads];         // Array to store thread identifiers
    ThreadData thread_data[max_threads];    // Array to store data for each thread
    double total_latency = 0;               // Variable to accumulate total latency

    /* Create threads to perform HTTP requests */
    for (int i = 0; i < max_threads; ++i) {
        thread_data[i].thread_id = i;
        thread_data[i].requests_per_thread = requests_per_thread;
        strncpy(thread_data[i].host, host, MAX_HOST_LENGTH);
        thread_data[i].host[MAX_HOST_LENGTH - 1] = '\0';
        pthread_create(&threads[i], NULL, performRequest, &thread_data[i]);
    }

    /* Wait for all threads to complete their execution */
    for (int i = 0; i < max_threads; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error: Failed to join thread %d\n", i);
        } else {
            total_latency += thread_data[i].total_latency;
        }
    }

    double avg_latency = total_latency / (max_threads * requests_per_thread);
    double throughput = (max_threads * requests_per_thread) / total_latency;

    char avg_latency_str[30], total_requests_str[30], throughput_str[30];

    /* Format the numerical values as strings */
    snprintf(avg_latency_str, sizeof(avg_latency_str), "%.5f seconds", avg_latency);
    snprintf(total_requests_str, sizeof(total_requests_str), "%d", max_threads * requests_per_thread);
    snprintf(throughput_str, sizeof(throughput_str), "%.2f requests/sec", throughput);

    printf("+--------------------------------+----------------------------+\n");
    printf("| %-30s | %-26s |\n", "Metric", "Value");
    printf("+--------------------------------+----------------------------+\n");
    printf("| %-30s | %-26s |\n", "Average Latency", avg_latency_str);
    printf("| %-30s | %-26s |\n", "Total Requests", total_requests_str);
    printf("| %-30s | %-26s |\n", "Throughput", throughput_str);
    printf("+--------------------------------+----------------------------+\n");

    return 0;
}
