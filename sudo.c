#include <windows.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command-to-run>\n", argv[0]);
        return 1;
    }

    // Construct the command string with "/k " prefix
        // use "/c " if the cmd window should disappear after command completed
    char pre_command[4] = "/k ";

    char *command = malloc(strlen(pre_command) + strlen(argv[1]) + 1);
    strcpy(command, pre_command);
    strcat(command, argv[1]);
    
    // Create cmd process
    SHELLEXECUTEINFO shExecInfo;
    ZeroMemory(&shExecInfo, sizeof(shExecInfo));
    
    shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shExecInfo.fMask = SEE_MASK_DEFAULT;
    shExecInfo.hwnd = NULL;
    shExecInfo.lpVerb = "runas";  // Run as admin
    shExecInfo.lpFile = "cmd.exe";
    shExecInfo.lpParameters = command; 
    shExecInfo.lpDirectory = NULL;
    shExecInfo.nShow = SW_SHOW;  // Show the command prompt window
    shExecInfo.hInstApp = NULL;

    // Execute command
    if (!ShellExecuteEx(&shExecInfo)) {
        printf("Error executing command. Error code: %lu\n", GetLastError());
        return 1;
    }

    return 0;
}
