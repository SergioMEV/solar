# Solar Shared Editor

Solar is a command line shared text editor that is the culminating project for Grinnell College's Operating Systems course. This program allows users to collaborate and make changes to the file in real-time. This editor will have a more stringent quality control and conflict-prevention mechanism than Google Docs/Microsoft Word. Users will only be able to edit/create one line at a time and will have exclusive control over that line for the length of their edit. This feature prevents conflicts from arising when multiple users simultaneously edit a shared file. The program is written in C and employs principles such as distributed systems, file systems, and thread synchronization to provide a shared editing system with a file exporting function.

## Demo
Insert images showing inputs and outputs

## How it works?

## Instruction
### 1. Compilation 
```
make
```

### 2. Run server
```
./server
```

### 3. Join session
```
$ ./client <server name> <port> <user name>
```

### 4. Export chat as a txt file

### 5. Quit session
Kill the terminal to exit the session. Note that if server quits, all the clients will also quit. 
