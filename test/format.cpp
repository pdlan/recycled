#include <iostream>
#include <string>
#include <recycled/format.h>

using namespace std;
using namespace recycled::format;

int main() {
    cout << "Here are %d %s in %f feet water\n" % _(100, "ducks", 123.456789);
    cout << "They have more than %lld cells\n" % _(10000000000000LL);
    cout << string("Now we have %d of them\n") % 123;
    return 0;
}