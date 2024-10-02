<p align="center">
  <img src="https://img.icons8.com/?size=512&id=55494&format=png" width="20%" alt="BLUETOOTH-CONNECTOR-LINUX-logo">
</p>
<p align="center">
    <h1 align="center">BLUETOOTH-CONNECTOR-LINUX</h1>
</p>
<p align="center">
    <em><code>❯ Bt</code></em>
</p>
<p align="center">
	<img src="https://img.shields.io/github/license/suaybdemir/bluetooth-connector-linux?style=flat&logo=opensourceinitiative&logoColor=white&color=f1f1f1" alt="license">
	<img src="https://img.shields.io/github/last-commit/suaybdemir/bluetooth-connector-linux?style=flat&logo=git&logoColor=white&color=f1f1f1" alt="last-commit">
	<img src="https://img.shields.io/github/languages/top/suaybdemir/bluetooth-connector-linux?style=flat&color=f1f1f1" alt="repo-top-language">
	<img src="https://img.shields.io/github/languages/count/suaybdemir/bluetooth-connector-linux?style=flat&color=f1f1f1" alt="repo-language-count">
</p>
<p align="center">
		<em>Built with the tools and technologies:</em>
</p>
<p align="center">
	<img src="https://img.shields.io/badge/JavaScript-F7DF1E.svg?style=flat&logo=JavaScript&logoColor=black" alt="JavaScript">
	<img src="https://img.shields.io/badge/C-A8B9CC.svg?style=flat&logo=C&logoColor=black" alt="C">
	<img src="https://img.shields.io/badge/JSON-000000.svg?style=flat&logo=JSON&logoColor=white" alt="JSON">
</p>

<br>

#####  Table of Contents

- [ Overview](#-overview)
- [ Features](#-features)
- [ Repository Structure](#-repository-structure)
- [ Modules](#-modules)
- [ Getting Started](#-getting-started)
    - [ Prerequisites](#-prerequisites)
    - [ Installation](#-installation)
    - [ Usage](#-usage)
    - [ Tests](#-tests)
- [ Project Roadmap](#-project-roadmap)
- [ Contributing](#-contributing)
- [ License](#-license)
- [ Acknowledgments](#-acknowledgments)

---

##  Overview

<code>❯ <p align="center"><img src="https://i.ibb.co/nQwrKRg/b-connector.png" width="390" height="190" 
title="cli"></p></code>

---

##  Features

<code>❯ Scan , connect,disconnect,pair any device ( allows you to use your phone's speaker )</code>

---

##  Repository Structure

```sh
└── bluetooth-connector-linux/
    ├── Entities
    │   ├── connection.h
    │   ├── pair.h
    │   └── scanBluetoothDevices.h
    ├── Makefile
    ├── README.md
    ├── con.h
    ├── con.h.gch
    ├── connectors.h
    ├── db.txt
    ├── index.js
    ├── main.cpp
    ├── mainScreen.h
    ├── output
    ├── package-lock.json
    ├── package.json
    └── yarn-error.log
```

---

##  Modules

<details closed><summary>Current</summary>

| File | Summary |
| --- | --- |
| [main.cpp](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/main.cpp) | <code>❯ main</code> |
| [mainScreen.h](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/mainScreen.h) | <code>❯ mainScreen.h</code> |
| [Makefile](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/Makefile) | <code>❯ Makefile</code> |
| [con.h](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/con.h) | <code>❯ con.h</code> |
| [connectors.h](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/connectors.h) | <code>❯ connectors</code> |
| [db.txt](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/db.txt) | <code>❯ Database</code> |

</details>

<details closed><summary>Entities</summary>

| File | Summary |
| --- | --- |
| [connection.h](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/Entities/connection.h) | <code>❯ connection.h</code> |
| [scanBluetoothDevices.h](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/Entities/scanBluetoothDevices.h) | <code>❯ scanBluetoothDevices.h</code> |
| [pair.h](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/Entities/pair.h) | <code>❯ pair.h</code> |

</details>

---

##  Getting Started

###  Prerequisites

**CPP**: `11`

###  Installation

Build the project from source:

1. Clone the bluetooth-connector-linux repository:
```sh
❯ git clone https://github.com/suaybdemir/bluetooth-connector-linux
```

2. Navigate to the project directory:
```sh
❯ cd bluetooth-connector-linux
```

3. Install the required dependencies:
```sh
❯ gcc -o myapp main.c
```

###  Usage

To run the project, execute the following command:

```sh
❯ ./myapp
```

---

##  Contributing

Contributions are welcome! Here are several ways you can contribute:

- **[Report Issues](https://github.com/suaybdemir/bluetooth-connector-linux/issues)**: Submit bugs found or log feature requests for the `bluetooth-connector-linux` project.
- **[Submit Pull Requests](https://github.com/suaybdemir/bluetooth-connector-linux/blob/main/CONTRIBUTING.md)**: Review open PRs, and submit your own PRs.
- **[Join the Discussions](https://github.com/suaybdemir/bluetooth-connector-linux/discussions)**: Share your insights, provide feedback, or ask questions.

<details closed>
<summary>Contributing Guidelines</summary>

1. **Fork the Repository**: Start by forking the project repository to your github account.
2. **Clone Locally**: Clone the forked repository to your local machine using a git client.
   ```sh
   git clone https://github.com/suaybdemir/bluetooth-connector-linux
   ```
3. **Create a New Branch**: Always work on a new branch, giving it a descriptive name.
   ```sh
   git checkout -b new-feature-x
   ```
4. **Make Your Changes**: Develop and test your changes locally.
5. **Commit Your Changes**: Commit with a clear message describing your updates.
   ```sh
   git commit -m 'Implemented new feature x.'
   ```
6. **Push to github**: Push the changes to your forked repository.
   ```sh
   git push origin new-feature-x
   ```
7. **Submit a Pull Request**: Create a PR against the original project repository. Clearly describe the changes and their motivations.
8. **Review**: Once your PR is reviewed and approved, it will be merged into the main branch. Congratulations on your contribution!
</details>

<details closed>
<summary>Contributor Graph</summary>
<br>
<p align="left">
   <a href="https://github.com{/suaybdemir/bluetooth-connector-linux/}graphs/contributors">
      <img src="https://contrib.rocks/image?repo=suaybdemir/bluetooth-connector-linux">
   </a>
</p>
</details>

---
