#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <pcrecpp.h>
#include <cassert>
#include <cursesw.h>
#include <unordered_map>

std::unordered_map<std::string, std::string> m_colours;
int scr_width;

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

        while ( fldend < spec.length() && isalnum(spec[fldend++]) );
        if (fldend != spec.length() ) fldend--;

        res = spec.substr(fldpos+fld_len, fldend-fldpos-fld_len);
    }

    return res;
}

int get_width(const std::string spec, const std::string fieldname)
{
    
    std::string t;
    int val = -1;

    t = get_field(spec, fieldname);
    if ( t != "")
    {
        if (t.back() == '%')
            val = atoi(t.c_str())*scr_width/100;
        else
            val = atoi(t.c_str());
    }
    return val;
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
    size_t cur_pos = 0;

    options.set_utf8(true);

    pcrecpp::RE vars( "(.*?)\\$([a-zA-Z]+)\\{(.*?)\\}", options);

    /* For each token : 
     *   - beg is the part before the var
     *   - var is the variable name
     *   - spec is the part between {}
     */
    while( vars.PartialMatch(input, &beg, &var, &spec) )
    {
        int min = -1;
        int max = -1;
        const size_t match_len = 1 + var.length() + 1 + spec.length() + 1;
        std::string color;

        cur_pos += beg.length() + match_len;
        /* Keep the beginning */
        result += beg;
        input.erase(0, beg.length()+match_len);

        min = get_width(spec, "min");
        max = get_width(spec, "max");

        color = get_field(spec, "color");
        if ( color != "" )
            color = m_colours[color];

        std::string value = headers[var];

        std::cout << "DEBUG : var:" <<  var << " spec:'" << spec << "'\n";
        std::cout << "DEBUG : $" <<  var << "='" << value << "'\n";
        std::cout << "DEBUG : min=" <<  min << " max=" << max << "\n";

        /* Length logic :
        * -1 means "Use string length"
        */

        if (color != "")
            result += color;

        /* Make sure min is not greater than max */
        if (max > 0 && min > max)
            min = max;

        std::string fitstr;
        if (min > 0 and value.length() < min )
        {
            fitstr =  value + std::string(min-value.length(), ' ');
        } else  if ( value.length() > max )
                {
                    fitstr = value.substr(0, max);
                }
                else
                    fitstr = value;
        result += fitstr;

        if (color != "")
            result +=  "\033[22;0m";
    }

    result += input;
    std::cout << "DEBUG : result : " << result << "\n";

    return( result );
}



/**
 * Simple test.
 */
int main( int argc, char *argv[] )
{
    
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    scr_width = w.ws_col;

    m_colours[ "black" ] = "\033[22;30m";
    m_colours[ "red"  ] = "\033[22;31m";
    m_colours[ "green" ] = "\033[22;32m";
    m_colours[ "yellow" ] = "\033[22;33m";
    m_colours[ "blue" ] = "\033[22;34m";
    m_colours[ "magenta" ] = "\033[22;35m";
    m_colours[ "cyan" ] = "\033[22;36m";
    m_colours[ "white" ] = "\033[22;37m";

    std::string test = expand_var("$FROM{color:red min:20%}|$SUBJECT{color:blue}|", {{"FROM", "me"},{"SUBJECT", "test"}});

    std::cout << test << "\n";

    assert( expand_var( " $SUBJECT{min:10 max:20}", {{"SUBJECT","steve"}} ) == " steve     " );
    assert( expand_var( "$SUBJECT{max:3}", {{"SUBJECT", "12345"}} ) == "123" );

    return 0;
}
