#ifndef ARG_PARSER_H
#define ARG_PARSER_H

/**
 * @brief Parse a command string into an argv-style array
 * @param cmd_string Command string to parse (e.g., "-u http://... -r /api")
 * @param args_out Output pointer to receive the allocated args array
 * @return Number of arguments parsed, or 0 on error
 * @note Caller must free the returned array using free_args()
 */
int parse_command_args(const char *cmd_string, char ***args_out);

/**
 * @brief Free an args array allocated by parse_command_args
 * @param args The args array to free
 */
void free_args(char **args);

/**
 * @brief Update fetcher args to use the current date for elprisetjustnu API
 * @param args The args array to update in-place
 * @param out_date_str Output pointer to receive date string (YYYY-MM-DD)
 * @return 0 on success, -1 on error
 */
int update_fetcher_args_date(char **args, char *out_date_str, const char *zone);

#endif /* ARG_PARSER_H */
