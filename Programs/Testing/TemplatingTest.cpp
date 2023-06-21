#include <HTML/HTMLTemplate.hpp>
#include <exception>
#include <iostream>

void TestWith(HTML::HTMLTemplate& tpl, std::vector<std::string>&& strings);

int main()
{
    HTML::HTMLTemplate test("HTMLTemplates/test.txt");
    std::cout << "Testing on small file...\n"
              << " ===== TEMPLATE =====\n"
              << test.GetTemplate() << std::endl;

    TestWith(test, {"AAA", "BBB", "CCC", "DDD", "EEE"});
    TestWith(test, {"AAA", "BBB", "CCC"});
    TestWith(test, {});
    TestWith(test, {"AAA", "BBB", "CCC", "DDD"});

    HTML::HTMLTemplate test2("HTMLTemplates/formTemplate.html");
    std::cout << "Testing on large file...\n"
              << " ===== TEMPLATE =====\n"
              << test2.GetTemplate() << std::endl;
    TestWith(test2, {"bleck"});
    TestWith(test2, {"Jeremiah the Bullfrog", "jbf@gmail.com", "123 Swamp Apt. 227", "4/16/1954",
                     "12345678", "123-45-678"});

    return 0;
}

void TestWith(HTML::HTMLTemplate& tpl, std::vector<std::string>&& strings)
{
    std::cout << "Attempting to substitute with:\n    ";
    for (auto s : strings)
        std::cout << s << ", ";
    std::cout << std::endl;

    try
    {
        tpl.Substitute(strings);
        std::string output = tpl.GetSubstitution();
        std::cout << " ===== OUTPUT =====\n" << output << "\n ===== END OUTPUT =====" << std::endl;
    }
    catch (std::exception ex)
    {
        std::cout << "Caught exception: " << ex.what() << std::endl;
    }
}