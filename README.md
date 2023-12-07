# bench ⚡

## Introduction 🌟

bench is a practical and efficient proof of concept (POC) tool designed for benchmarking HTTP servers. It's tailored to simulate server load by sending concurrent HTTP requests, thus assessing server performance in terms of response times and throughput.

## Project Overview 📖

This POC is ideal for those interested in:

-   Learning about multi-threaded processing.
-   Exploring socket programming and HTTP communication in C.
-   Gaining insights into server performance metrics like response time and throughput.

## Features 🛠️

-   **Concurrent HTTP Requests:** Capable of sending multiple HTTP GET requests simultaneously to test server load handling.
-   **Multi-Threaded Approach:** Employs POSIX threads for concurrent processing, effectively simulating a high-traffic environment.
-   **Performance Metrics:** Calculates and displays average latency and throughput, offering a basic overview of server performance under simulated load.
-   **Flexible Configuration:** Users can set the target host and define the number of requests each thread should make.

## Getting Started 🚀

-   **Prerequisites:** C Compiler (gcc) and Make for building the application.
-   **Clone the Repository:** Begin by cloning the source code from the repository.
-   **Compile the Application:** Use the `make` command to compile the tool.
-   **Run the Benchmark:** Execute the tool using `./bench <host> [<requests_per_thread>].`

Example: `./bench www.google.com 8`

## Contributing 🤝

As a learning project, contributions are especially welcome. Whether you're correcting bugs, proposing new features, or improving documentation, your input is valuable.

Fork the repository, make your changes, and submit a pull request. Let's learn and grow together with this project!
