# Solar Shared Editor

[TOC]

## Introduction

Solar is a command line shared text editor that is the final project for Grinnell College's Operating Systems course. This program allows users to collaborate and make changes to the file in real-time. This editor will have a more stringent quality control and conflict-prevention mechanism than Google Docs/Microsoft Word. Users will only be able to edit/create one line at a time and will have exclusive control over that line for the length of their edit. This feature prevents conflicts from arising when multiple users simultaneously edit a shared file. The program is written in C and employs principles such as distributed systems, file systems, and thread synchronization to provide a shared editing system with a file exporting function.

## Technologies

- C
- LINUX environment
- Terminal/Command line

## Instruction

### 1. Create a file that you want to work on

This can be done manually in the folder or use the following command to create one using the terminal:

```
touch <filename>
```

### 2. Compilation

Complie the program using the following command:

```
$ make
```

### 3. Run server

To start running the server, run the following command with a filename that you created in the first step:

```
$ ./server <filename>
```

### 4. Join session

To start editing, connect to the server created using the following commands:

```
$ ./client <server name> <port> <user name>
```

Insert your IP address in <servername> if you are running this program on your personal computer.

An example running the program on one's own computer:

```
$ ./client localhost 99999 student
```

### 5. Client Controls

The following are the controls client users have to edit the file:

- `ENTER`: Modify the current line
- `I`: Insert a new line
- `D`: Delete the current line
- `A`: Create a new line at the end
- `X`: Overwrite the file contents to a new one
- `esc`: Exit session

### 6. Quit server

Press `control + c` to terminate the server.

## Demo





## Software Architecture 

![Architecture](README.assets/Architecture.jpg)

# Milestones

## Milestone 1: Infrustructure

### Task 1 user_info:

- define struct
- write constructors and util functions

### Task 2 file_system:

- define struct
- write constructors and util functions

### Task 3 logs

- define log struct 

## Milestone 2: UI

### Task 1 initialize ui

### Task 2 implement drivers for ui

- text-box driver 
- display driver 

## Milestone 3: network connection

- server 
- client 





