#include <libintl.h>
#include <locale.h>
#include <iostream>

int main ()
{
    setlocale(LC_ALL, "");
    bindtextdomain("i18n", ".");
    textdomain( "i18n");
    std::cout << gettext("hello, world!") << std::endl;
}
