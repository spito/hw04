#ifndef CONFIG_H
#define CONFIG_H

enum ConfigValueType {
    // From the first non-white character up to the last one on the line.
    CFG_STRING,
    // Only valid number can be condidered. 
    CFG_INTEGER,
    // All of {1, "true", "yes"} should be accepted as true.
    // All of {0, "false", "no"} should be accepted as false.
    CFG_BOOL
};

struct Config {
    /// TODO: implement
};

/** Read the config file
 *
 *  @param cfg The config structure.
 *  @param name The path to the config file.
 *  @return 0 in case of success
 *          1 in case the file cannot be opened
 *          2 in case the format of the config file is wrong
 */
int configReader(struct Config *cfg, const char *name); 

/** Get the config value
 *
 *  @param cfg The config structure.
 *  @param section The section of the config file.
 *  @param key The key of the section of the config file.
 *  @param type The type of the value.
 *  @param value The output parameter for loading value.
 *  @return 0 in case of success
 *          1 in case the section name is not found
 *          2 in case the key is not found
 *          3 in case the value cannot be converted into the desired type
 */
int configValue(const struct Config *cfg,
                const char *section,
                const char *key,
                enum ConfigValueType type,
                void *value);

/** Release all resources hold by Config structure. 
 *
 *  @param cfg The config structure.
 */
void configCleaner(struct Config *cfg);

#endif
