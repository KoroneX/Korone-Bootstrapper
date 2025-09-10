# Korone Bootstrapper

This repository contains the source code for the [Korone Bootstrapper](https://pekora.zip).

We are open sourcing this project to demonstrate transparency. Some users have mistakenly labeled Korone as a **RAT (Remote Access Trojan)** this code proves that the bootstrapper is completely safe.

---

## Requirements

Before building, make sure you have the following installed:

1. **[Visual Studio 2022](https://visualstudio.microsoft.com/downloads/)**
2. **[vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-vs?pivots=shell-powershell)**

---

## Building

1. Clone the repository and open the solution in **Visual Studio 2022**.
2. Select the **configuration**: `Release | x86` or `Release | x64`.
3. Build the `BootstrapperClient` project.
4. Thats it! The compiled binary will be ready in your output directory.