#include "archium.h"

static void execute_command(const char *command, const char *log_message) {
    if (system(command) != 0) {
        fprintf(stderr, "\033[1;31mError: Command failed: %s\033[0m\n", command);
    }
    if (log_message) {
        log_action(log_message);
    }
}

static void get_user_input(char *buffer, const char *prompt) {
    rl_attempted_completion_function = command_completion;
    get_input(buffer, prompt);
    rl_attempted_completion_function = NULL;
}

ArchiumError handle_command(const char *input, const char *package_manager) {
  log_action(input);

  if (!is_valid_command(input)) {
      fprintf(stderr, "Error: Invalid command. Type 'h' for help.\n");
      return ARCHIUM_ERROR_INVALID_INPUT;
  }

  if (strcmp(input, "h") == 0 || strcmp(input, "help") == 0) {
      display_help();
      return ARCHIUM_SUCCESS;
  }

  if (strcmp(input, "q") == 0 || strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
      printf("Exiting Archium.\n");
      exit(0);
  }

  if (strcmp(input, "u") == 0) {
      update_system(package_manager, NULL);
  } else if (strncmp(input, "u ", 2) == 0) {
      update_system(package_manager, input + 2);
  } else if (strcmp(input, "i") == 0) {
      char packages[MAX_INPUT_LENGTH];
      get_user_input(packages, "Enter package names to install: ");
      install_package(package_manager, packages);
  } else if (strcmp(input, "r") == 0) {
      char packages[MAX_INPUT_LENGTH];
      get_user_input(packages, "Enter package names to remove: ");
      remove_package(package_manager, packages);
  } else if (strcmp(input, "p") == 0) {
      char packages[MAX_INPUT_LENGTH];
      get_user_input(packages, "Enter package names to purge: ");
      purge_package(package_manager, packages);
  } else if (strcmp(input, "c") == 0) {
      clean_cache(package_manager);
  } else if (strcmp(input, "cc") == 0) {
      clear_build_cache();
  } else if (strcmp(input, "o") == 0) {
      clean_orphans(package_manager);
  } else if (strcmp(input, "lo") == 0) {
      list_orphans();
  } else if (strcmp(input, "s") == 0) {
      char package[MAX_INPUT_LENGTH];
      get_user_input(package, "Enter package name to search: ");
      search_package(package_manager, package);
  } else if (strcmp(input, "l") == 0) {
      list_installed_packages();
  } else if (strcmp(input, "?") == 0) {
      char package[MAX_INPUT_LENGTH];
      get_user_input(package, "Enter package name to show info: ");
      show_package_info(package_manager, package);
  } else if (strcmp(input, "cu") == 0) {
      check_package_updates();
  } else if (strcmp(input, "dt") == 0) {
      char package[MAX_INPUT_LENGTH];
      get_user_input(package, "Enter package name to view dependencies: ");
      display_dependency_tree(package_manager, package);
  }

  return ARCHIUM_SUCCESS;
}

ArchiumError handle_exec_command(const char *command, const char *package_manager) {
    if (!command || !package_manager) {
        return ARCHIUM_ERROR_INVALID_INPUT;
    }

    log_action(command);

    return handle_command(command, package_manager);
}

void update_system(const char *package_manager, const char *package) {
    char command[COMMAND_BUFFER_SIZE];
    if (package) {
        snprintf(command, sizeof(command), "%s -S %s", package_manager, package);
        printf("\033[1;34mUpgrading package: %s\033[0m\n", package);
    } else {
        snprintf(command, sizeof(command), "%s -Syu --noconfirm", package_manager);
        printf("\033[1;34mUpgrading system...\033[0m\n");
    }
    execute_command(command, "System updated");
}

void clear_build_cache() {
    printf("\033[1;34mClearing package build cache...\033[0m\n");
    execute_command("rm -rf $HOME/.cache/yay", "Cleared package build cache");
}

void list_orphans() {
    printf("\033[1;34mListing orphaned packages...\033[0m\n");
    execute_command("pacman -Qdt", NULL);
}

void install_package(const char *package_manager, const char *packages) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -S %s", package_manager, packages);
    printf("\033[1;34mInstalling packages: %s\033[0m\n", packages);
    execute_command(command, "Installed packages");
}

void remove_package(const char *package_manager, const char *packages) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -R %s", package_manager, packages);
    printf("\033[1;34mRemoving packages: %s\033[0m\n", packages);
    execute_command(command, "Removed packages");
}

void purge_package(const char *package_manager, const char *packages) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Rns %s", package_manager, packages);
    printf("\033[1;34mPurging packages: %s\033[0m\n", packages);
    execute_command(command, "Purged packages");
}

void clean_cache(const char *package_manager) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Sc --noconfirm", package_manager);
    printf("\033[1;34mCleaning package cache...\033[0m\n");
    execute_command(command, "Cache cleaned");
}

void clean_orphans(const char *package_manager) {
  char orphan_check_command[COMMAND_BUFFER_SIZE];
  snprintf(orphan_check_command, sizeof(orphan_check_command), "pacman -Qdtq");

  FILE *fp = popen(orphan_check_command, "r");
  if (!fp) {
      fprintf(stderr, "\033[1;31mError: Failed to check for orphaned packages.\033[0m\n");
      return;
  }

  char orphaned_packages[COMMAND_BUFFER_SIZE] = {0};
  if (!fgets(orphaned_packages, sizeof(orphaned_packages), fp)) {
      pclose(fp);
      printf("\033[1;32mNo orphaned packages found.\033[0m\n");
      return;
  }
  pclose(fp);

  orphaned_packages[strcspn(orphaned_packages, "\n")] = '\0';

  char command[COMMAND_BUFFER_SIZE];
  snprintf(command, sizeof(command), "%s -Rns %s", package_manager, orphaned_packages);
  printf("\033[1;34mCleaning orphaned packages...\033[0m\n");
  execute_command(command, "Orphaned packages cleaned");
}

void search_package(const char *package_manager, const char *package) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Ss %s", package_manager, package);
    printf("\033[1;34mSearching for package: %s\033[0m\n", package);
    execute_command(command, NULL);
}

void list_installed_packages(void) {
    printf("\033[1;34mListing installed packages...\033[0m\n");
    execute_command("pacman -Qe", NULL);
}

void show_package_info(const char *package_manager, const char *package) {
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s -Si %s", package_manager, package);
    printf("\033[1;34mShowing information for package: %s\033[0m\n", package);
    execute_command(command, NULL);
}

void check_package_updates(void) {
    printf("\033[1;34mChecking for package updates...\033[0m\n");
    execute_command("pacman -Qu", "Checked for updates");
}

void display_dependency_tree(const char *package_manager, const char *package) {
    (void)package_manager;
    char command[COMMAND_BUFFER_SIZE];
    snprintf(command, sizeof(command), "pactree %s", package);
    printf("\033[1;34mDisplaying dependency tree for package: %s\033[0m\n", package);
    execute_command(command, NULL);
}