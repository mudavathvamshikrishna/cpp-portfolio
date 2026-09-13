# cpp-portfolio
My C++ projects for internship preparation

Hi, I'm Vamshi Krishna, a 2nd-year BTech student.  
This repo tracks my C++ projects as I prepare for software engineering internships.

## Projects

### 1. Multi-threaded File Downloader (In Progress)

**Goal:** Build a command-line tool in C++ that downloads a file from a URL using multiple threads.

**Planned features:**
- [ ] Accept URL and output filename from command line
- [ ] Download a file over HTTP/HTTPS
- [ ] Split the file into chunks and download them concurrently using `std::thread`
- [ ] Show a simple progress indicator
- [ ] Support resuming an interrupted download
- [ ] Handle invalid URLs and network errors
- [ ] Add basic tests

**Tech stack:** C++17, CMake, libcurl, `std::thread`, `std::mutex`, file I/O

**Why I'm building this:**  
To learn concurrency, networking, memory management, and file I/O in C++ — skills that are useful for internship interviews.


