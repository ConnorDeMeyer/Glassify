# Glassify
Glassify is a simple customizable Header-Only Reflection System for C++.
It allows the user to easily reflect upon types wherever they are inside of the codebase. The reflection data is generated before the main function is executed. The developer can choose which data gets stored inside of the reflection system by adding code to the config files.

## Requisites
C++ 20

In case you're using a static library: add the `/WHOLEARCHIVE`/`--whole-archive` linker flag for MSVC/GCC. In case you get LNK2005 errors, consider using the `/FORCE:MULTIPLE`/`--allow-multiple-definition` linker flag.

## Installation
Simply include the `include` folder into your project.
`#include "glassify.h"` should be used at the top of any file that wishes to use the reflection system.

### Example directory
The example directory should not be included into an existing project and only exists for testing and providing basic examples.

## Customizing Glassify
There are various features/Addons that can be toggled on that are off by default to save on compilation time:
To enable/disable any of these features, the respective macros must be defined at the top of the `glas_decl.h` file.

- `GLAS_STORAGE`: Used for constructing and destructing instance of types. [More Info](#Storage).
- `GLAS_SERIALIZATION_BINARY`: Used for serializing and deserializing type instances to binary streams. [More Info](#Serialization).
- `GLAS_SERIALIZATION_JSON`: Used for Serializing and deserializing type instances to JSon format. [More Info](#Serialization).
- `GLAS_SERIALIZATION_YAML`: Used for Serializing and deserializing type instances to YAML format. [More Info](#Serialization).

### Custom member variable/function properties
There are 2 enum class at the top of the `glas_decl.h` file:
1. `MemberProperties`: Customizable properties for member variables.
2. `FunctionProperties`: Customizable properties for functions and methods.

## Usage

### Reflecting Types
Types can be added to the reflection system by using the `GLAS_TYPE()` macro:
```cpp
struct Vector
{
	float X{}, Y{}, Z{};
};

GLAS_TYPE(Vector);
```

#### Type Identifier
A `glas::TypeId` is a class containing a hash that is unique for each Type. The hash is the same between program invokations as long as the Name of the type does not change.
The `TypeId` has a function called `GetInfo()` which will return the type info associated with the TypeId (if the type is registered to the reflection system).

You can get a `TypeId` by using the function `Glas::TypeId::Create<TYPE>()`, where TYPE is the name of the type you want to create a `TypeId` for.

You may access all registered `TypeId` and their perspective `TypeInfo` using the function:
```cpp
glas::GetAllTypeInfo()
```

#### Type Info
`glas::TypeInfo` is a struct containing the type information that can normally only be accessed at compile time. the information in this struct can be customized inside of the `glas_decl.h` file.


### Reflecting Member Variables
Member variables can be added to the reflection system by using the `GLAS_MEMBER(CLASS, MEMBER)`. `CLASS` is the owning class type and `MEMBER` is the name of the member variable.
```cpp
struct Vector
{
	float X{}, Y{}, Z{};
};

GLAS_MEMBER(Vector, X);
GLAS_MEMBER(Vector, Y);
GLAS_MEMBER(Vector, Z);
```

#### Reflecting Private Member Variables
Private member variables can be reflected with the `GLAS_PRIVATE_MEMBER(CLASS, MEMBER)` macro. This macro *cannot* be placed inside of a namespace.

#### Variable Identifier
Variables can be identified using a `glas::VariableId`. This type holds a `glas::TypeId` and keeps track of the modifying factors like if it is _const_, _volatile_, _reference_, _r value reference_, _pointers_, _Array size_. This information can be freely queried and modified depending on the use of the type. It also contains a `ToString` method that allows for better representation of the variable type.

### Reflecting Functions
Functions and member functions can be reflected using Glassify. using the macros `GLAS_FUNCTION(Function)` and `GLAS_MEMBER_FUNCTION(Class, MemberFunction)` for functions and member functions.

Each reflected _function/member function_ will be assigned a unique _ID_ that stays the same between sessions as long as the function signature does not change (no name change and no parameter change). The identifier gets constructed from the has of the name, parameters and, in case of a member function, the class it belongs to.

Yo are able to get the ID of a _function/member function_ using the macros `GLAS_FUNCTION_ID(Function)` and `GLAS_MEMBER_FUNCTION_ID(Class, Function)`.

```cpp
int AnyFunction(double AnyParameters);

class AnyClass
{
public:
	void AnyMemberFunction();
}

GLASS_FUNCTION(AnyFunction);
GLASS_MEMBER_FUNCTION(AnyClass, AnyMemberFunction);
```

#### Function Information
The information that gets stored for any function is the following:
 - Function Address
 - Return Type
 - Name
 - Hash of Return Type and Parameter Types
 - Parameter Types
 - Properties

#### Function Casting
It is possible to try and dynamically cast any address to a specific function type using the `Cast` member function. If the cast is successful it will return a valid pointer, if unsuccessful it will return `nullptr`.

#### Function Calling
Using the `TypeTuple` class from the [Storage](#storage) addon it is possible to call a function at runtime using data that is generated at runtime given that the parameters share the same types as the variables inside the `TypeTuple`.

### Reflecting Parent-Child relation ships
The relation ship between a parent and child class can also be registered into the reflection system by using the Macro `GLAS_CHILD`.

```cpp
class BaseClass
{};

class ChildClass : public BaseClass
{};

GLAS_CHILD(BaseClass, ChildClass);
```

### Dependencies

Some Types may need to rely on other types in order to maintain functionality. For examples: If we want to serialize an `std::vector<int>` than we need to register both the `std::vector<int>` and `int` to the serialization to work correctly. In this case the `std::vector<int>` depends on `int` for the type to work correctly.

Using the type `Glas::AddDependency` we can define this relationship and automatically add any type which is depended on by another class, example:
```cpp
namespace glas
{
	template <typename T>
	struct AddDependency<std::vector<T>>
	{
		inline static AutoRegisterTypeOnce<T> RegisterValue{};
	};

	template <>
	struct AddDependency<Transform>
	{
		inline static AutoRegisterTypeOnce<ComponentBase> RegisterComponent{};
	};
}
```
Please look inside of `glas_dependencies.h` for more examples. These `AddDependency` structs can be created anywhere inside of the project.

# Addons
Addons are useful features that come with Glassify that can be completely removed from the project if the developer has no need for them

## Binary Serialization

This features allows the user to serialize a type to Binary. The `#define GLAS_SERIALIZATION_BINARY` macro must be defined inside of the `glas_decl.h` file. In order to serialize member variables, the `GLAS_MEMBER` macro must be used to register the member variables into the reflection system.

### Usage

```cpp
glas::Serialization::SerializeBinary(std::ostream& stream, const TYPE& value);
glas::Serialization::DeserializeBinary(std::istream& stream, TYPE& value);
```
### Custom Serializer
```cpp
template <>
struct BinarySerializer<TYPE>
{
	static void Serialize(std::ostream& stream, const TYPE& value);
	static void Deserialize(std::istream& stream, TYPE& value);
};
```

## JSon Serialization

This features allows the user to serialize a type to JSon. The `#define GLAS_SERIALIZATION_JSON` macro must be defined inside of the `glas_decl.h` file. In order to serialize member variables, the `GLAS_MEMBER` macro must be used to register the member variables into the reflection system.

### Usage

```cpp
glas::Serialization::SerializeJSon(std::ostream& stream, const TYPE& type);
glas::Serialization::DeserializeJSon(std::istream& stream, TYPE& value);
```
### Custom Serializer
```cpp
template <>
struct JSonSerializer<TYPE>
{
	static void Serialize(rapidjson::Value& jsonVal, const TYPE& value, RapidJsonAllocator& allocator);
	static void Deserialize(rapidjson::Value& jsonVal, TYPE& value);
};
```

## YAML Serialization

This features allows the user to serialize a type to YAML. The `#define GLAS_SERIALIZATION_YAML` macro must be defined inside of the `glas_decl.h` file. In order to serialize member variables, the `GLAS_MEMBER` macro must be used to register the member variables into the reflection system.

### Usage

```cpp
glas::Serialization::SerializeYaml(std::ostream& stream, const TYPE& type);
glas::Serialization::DeserializeYaml(std::istream& stream, TYPE& value);
```
### Custom Serializer
```cpp
template<>
struct convert
{
    static Node encode(const TYPE& value);

    static bool decode(const Node& node, TYPE& rhs);
};
```

## Storage

This feature allows the user to store and initialize instances of types at runtime using only the `glas::TypeId`. The `#define GLAS_STORAGE` macro must be defined inside of the `glas_decl.h` file.

### Settings


### Type Storage
Similarly to an std::unique_ptr, the `TypeStorage` class allows for the instanciation of any class that has been added to the reflection system. The class will instanciate a given type on the Heap and is responsible for safely destroying them.

### Shared Type Storage

Similarly to an std::shared_ptr, the `SharedTypeStorage` class allows instanciation of any class that has been added to the reflection system and can be easily copied and shared without the type instance going out of scope.

### Weak Type Storage

Similarly to an std::weak_ptr, the `WeakTypeStorage` class needs to `SharedTypeStorage` to initialize and will hold a reference to that instance until there are no more shared type storages left.

### Type Vector

Similarly to an std::vector, the `TypeVector` stores a contigious array of instances of a type. this vector can be freely added to, removed from and queried. There are also iterator that can be used to iterator over the elements. The whole vector is typles and will store only data. The user is responsible for interpreting the data.

### Type Tuple
The `TypeTuple` class works similarly to a `std::tuple` class but typeless. It stores multiple instances of types and keeps track of the types inside using an array of `VariableId`s. It can be used to call functions and can be serialized too.

