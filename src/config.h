#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

#include <vector>
#include <string>


/**
 * Our config-type can contain "string" or "array" values.
 *
 * Setup a type for that.
 */
typedef enum
{ CONFIG_UNKNOWN, CONFIG_STRING, CONFIG_ARRAY } configType;


/**
 * This is the struct which holds a single configuration value.
 *
 * The value might be a string, or an array of strings.
 */
struct CConfigEntry
{
  /**
   * The name of the configuration-option.
   */
    std::string * name;

  /**
   * The type of the configuration-option: STRING vs ARRAY
   */
    configType type;

  /**
   * The actual value of this entry, stored as a union.
   */
    union
    {
	std::string * str;
	std::vector < std::string > *array;
    } value;

};



/**
 * Singleton to hold configuration variables.
 */
class CConfig
{
  private:
    CConfig ();
    ~CConfig ();

  public:

  /**
   * Instance accessor - this is a singleton.
   */
    static CConfig *instance ();

  /**
   * Get the value associated with a name.
   */
    CConfigEntry *get (std::string name);

    /**
     * Get all the keys we know about.
     */
                 std::vector < std::string > keys ();

  /**
   * Set a configuration key to contain the specified value.
   */
    void set (std::string name, std::string value);

  /**
   * Set a configuration key to contain the specified array-value.
   */
    void set (std::string name, std::vector < std::string > entries);

  private:

  /**
   * Remove the value of the given key.
   */
    void delete_key (std::string key);

  private:

  /**
   * The actual storage.
   */
         std::vector < CConfigEntry * >m_entries;
};


#endif
