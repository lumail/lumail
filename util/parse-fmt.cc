#include <string>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <pcrecpp.h>
#include <cassert>
#include <unordered_map>


std::string get_field(const std::string spec, const std::string fieldname)
{
    const int fld_len = fieldname.length()+1;
    int fldpos;
    std::string res;

    fldpos = spec.find(fieldname+":");

    /** Make sure we have a real field definition */
    if ( fldpos != std::string::npos && fldpos+fld_len < spec.length() )
    {
        int fldend;

        /** skip "field:" */
        fldend = fldpos + fld_len;

        while ( fldend < spec.length() && isdigit(spec[fldend++]) );
        if (fldend != spec.length() ) fldend--;

        res = spec.substr(fldpos+fld_len, fldend-fldpos-fld_len);
    }

    return res;
}
/**
 * Expand a string such as '$SUBJECT{min:10 max:20 color:red} - Moi"
 * into something suitable.
 */
std::string expand_var(std::string input , std::unordered_map<std::string,std::string> headers)
{
    /**
     * Split into the token and the optional arguments.
     */
    pcrecpp::RE_Options options;
    std::string var, beg, spec, result;
    options.set_utf8(true);

    pcrecpp::RE vars( "(.*?)\\$([a-zA-Z]+)\\{(.*?)\\}", options);

    while( vars.PartialMatch(input, &beg, &var, &spec) )
    {
        int min = 0;
        int max = -1;
        const size_t match_len = 1 + var.length() + 1 + spec.length() + 1;
        
        result += beg;
        input.erase(0, beg.length()+match_len);

        min = atoi(get_field(spec, "min").c_str());
        max = atoi(get_field(spec, "max").c_str());

        std::string value = headers[var];

        std::cout << var << " " << spec << "\n";
        std::cout << min << " " << max << "\n";

        if (min > max)
            min = max;

        if (min > 0 and min > value.length() )
        {
            result += value + std::string(min-value.length(), ' ');
        } else if ( max < value.length() )
            {
                result += value.substr(0, max);
            }
    }


    std::cout << ":" << result << ":\n";
    return( result );
}



/**
 * Simple test.
 */
int main( int argc, char *argv[] )
{

    assert( expand_var( " $SUBJECT{min:10 max:20}", {{"SUBJECT","steve"}} ) == " steve     " );
    assert( expand_var( "$SUBJECT{max:3}", {{"SUBJECT", "12345"}} ) == "123" );

    return 0;
}
