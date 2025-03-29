#include "archium.h"

int check_archium_file(void) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "\033[1;31mError: Unable to get HOME environment variable.\033[0m\n");
        return 0;
    }

    char path[MAX_INPUT_LENGTH];
    if (snprintf(path, sizeof(path), "%s/.archium-use-paru", home) >= sizeof(path)) {
        fprintf(stderr, "\033[1;31mError: Path buffer overflow.\033[0m\n");
        return 0;
    }

    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int check_command(const char *command) {
    char cmd[COMMAND_BUFFER_SIZE];
    if (snprintf(cmd, sizeof(cmd), "command -v %s > /dev/null 2>&1", command) >= sizeof(cmd)) {
        fprintf(stderr, "\033[1;31mError: Command buffer overflow.\033[0m\n");
        return 0;
    }
    return (system(cmd) == 0);
}

int check_package_manager(void) {
    if (check_archium_file()) {
        return 2;  // paru
    }
    if (check_command("yay")) {
        return 1;  // yay
    }
    if (check_command("paru")) {
        return 2;  // paru
    }
    if (check_command("pacman")) {
        return 3;  // pacman
    }
    return 0;  // none
}

int check_git(void) {
    return check_command("git");
}

void install_git(void) {
    printf("\033[1;32mInstalling git...\033[0m\n");
    if (system("sudo pacman -S --noconfirm git") != 0) {
        fprintf(stderr, "\033[1;31mError: Failed to install git.\033[0m\n");
        exit(EXIT_FAILURE);
    }
}

void install_yay(void) {
    printf("\033[1;32mInstalling yay...\033[0m\n");
    if (system(
            "mkdir -p $HOME/.cache/archium/setup && "
            "cd $HOME/.cache/archium/setup && "
            "git clone https://aur.archlinux.org/yay-bin.git && "
            "cd yay-bin && "
            "makepkg -scCi && "
            "cd && "
            "rm -rf $HOME/.cache/archium/") != 0) {
        fprintf(stderr, "\033[1;31mError: Failed to install yay.\033[0m\n");
        exit(EXIT_FAILURE);
    }
    printf("\033[1;32mInstallation of yay is complete. Please restart your shell and relaunch Archium.\033[0m\n");
}

void prompt_install_yay(void) {
    char response[10];
    printf("\033[1;31mError: No suitable package manager is installed.\033[0m\n");
    sleep(1);
    printf("Do you want to install yay? (\033[1;32my\033[0m/\033[1;31mn\033[0m): ");

    if (!fgets(response, sizeof(response), stdin)) {
        fprintf(stderr, "\033[1;31mError: Failed to read input.\033[0m\n");
        exit(EXIT_FAILURE);
    }

    response[strcspn(response, "\n")] = '\0';

    if (strcasecmp(response, "y") == 0 || strcasecmp(response, "yes") == 0) {
        if (!check_git()) {
            install_git();
        }
        install_yay();
    } else {
        printf("\033[1;31mExiting Archium.\033[0m\n");
        exit(EXIT_FAILURE);
    }
}