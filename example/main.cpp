#include "config-base.h"

#include "alt-config/alt-config.h"
#include "toml++/toml.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace GenericConfig;

class AltConfig : public ConfigBase<AltConfig>
{
public:
    static ValuePtr ToValue(alt::config::Node& node)
    {
        if (node.IsScalar())
        {
	        try
	        {
                double value = node.ToNumber();
                return std::make_shared<Value>(value);
	        }
            catch(...)
            {
                try
                {
                    bool value = node.ToBool();
                    return std::make_shared<Value>(value);
                }
                catch(...)
                {
                    std::string value = node.ToString();
                    return std::make_shared<Value>(value);
                }
            }
        }
        if(node.IsList())
        {
            auto& list = node.ToList();
            Value::List value;
            value.reserve(list.size());
            for (auto& node : list)
            {
                value.push_back(ToValue(*node));
            }
            return std::make_shared<Value>(value);
        }
        if (node.IsDict())
        {
            auto& dict = node.ToDict();
            Value::Dict value;
            for (auto& pair : dict)
            {
                value.insert({ pair.first, ToValue(*pair.second) });
            }
            return std::make_shared<Value>(value);
        }
        return std::make_shared<Value>(nullptr);
    }

    static ValuePtr Parse(const std::string& input)
    {
        try
        {
            auto result = alt::config::Parser(input.data(), input.size()).Parse();
            return ToValue(result);
        }
        catch(...)
        {
            return std::make_shared<Value>(nullptr);
        }
    }
};

class TomlConfig : public ConfigBase<TomlConfig>
{
public:
    static ValuePtr ToValue(toml::node* node)
    {
	    if(node->is_string())
	    {
            return std::make_shared<Value>(node->as_string()->get());
	    }
        if(node->is_number())
        {
            double val;
            if (node->is_integer()) val = node->as_integer()->get();
            else val = node->as_floating_point()->get();
            return std::make_shared<Value>(val);
        }
        if(node->is_boolean())
        {
            return std::make_shared<Value>(node->as_boolean()->get());
        }
        if(node->is_array())
        {
            Value::List value;
            auto list = node->as_array();
            for(size_t i = 0; i < list->size(); i++)
            {
                value.push_back(ToValue(list->get(i)));
            }
            return std::make_shared<Value>(value);
        }
        if(node->is_table())
        {
            Value::Dict value;
            auto dict = node->as_table();
            for(auto it = dict->begin(); it != dict->end(); ++it)
            {
                value.insert({ it->first.data(), ToValue(&it->second) });
            }
            return std::make_shared<Value>(value);
        }
        return std::make_shared<Value>(nullptr);
    }

    static ValuePtr Parse(const std::string& input)
    {
        try 
        {
            auto data = toml::parse(input);
            return ToValue(data.as_table());
        }
        catch(...)
        {
            return std::make_shared<Value>(nullptr);
        }
    }
};

std::string ReadFile(const std::string& fileName)
{
    std::ifstream file(fileName);

    if (file.fail()) return {};
    std::ostringstream str{};
    file >> str.rdbuf();

    if (file.fail() && !file.eof()) return {};
    return str.str();
}

std::string altConfigFile = R"(
name: "Leon's Testserver"
host: 0.0.0.0
port: 7788
debug: true
modules: [
  'js-module'
]
js-module: {
  profiler: {
    port: 7799
  }
}
)";

std::string tomlConfigFile = R"(
name = "Leon's Testserver"
host = "0.0.0.0"
port = 7788
debug = true
modules = [ "js-module" ]
[js-module.profiler]
port = 7799
)";

void PrintConfigValues(Value& config)
{
    std::cout << config.Get("name").As<std::string>() << std::endl;
    std::cout << config["host"].As<std::string>() << std::endl;
    std::cout << config.Get("port").As<int>() << std::endl;
    std::cout << config.Get("debug").As<bool>() << std::endl;
    std::cout << config.Get("modules").Get(0).As<std::string>() << std::endl;
    std::cout << config.Get("js-module").Get("profiler").Get("port").As<int>() << std::endl;
}

int main(int, char**)
{
    {
        std::cout << "Parsing alt-config file..." << std::endl;
        std::shared_ptr<Value> config = AltConfig::Parse(altConfigFile);
        std::cout << config->ToString(true) << std::endl;
        PrintConfigValues(*config);
    }

    {
        std::cout << "Parsing toml file..." << std::endl;
        std::shared_ptr<Value> config = TomlConfig::Parse(tomlConfigFile);
        std::cout << config->ToString(true) << std::endl;
        PrintConfigValues(*config);
    }

	return 0;
}

