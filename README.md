# Glassify
Glassify is a simple customizable Header-Only Reflection System for C++.
It allows the user to easily reflect upon types wherever they are inside of the codebase. The reflection data is generated before the main function is executed. The developer can choose which data gets stored inside of the reflection system by adding code to the config files.

## Installation
Simply include the `include` folder into your project.
`#include "glassify.h"` should be used at the top of any file that wishes to use the reflection system.

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
Because private/protected member variables cannot be reached by other classes, we have to create a friend class that contains the `GLAS_MEMBER()` macro

```cpp
class GameObject final
{
private:
	Transform Transform{};
	std::string Name{ "None" };
	uint32_t Id{};

	friend struct RegisterGameObject;
};

struct RegisterGameObject final
{
	GLAS_MEMBER(GameObject, Name);
	GLAS_MEMBER(GameObject, Id);
	GLAS_MEMBER(GameObject, Transform);
};
```

### Type Identifier
A `glas::TypeId` is a class containing a hash that is unique for each Type. The hash is the same between program invokations as long as the Name of the type does not change.

You can get a `TypeId` by using the function `Glas::TypeId::Create<TYPE>()`, where TYPE is the name of the type you want to create a `TypeId` for.

You may access all registered `TypeId` and their perspective `TypeInfo` using the function:
```cpp
glas::GetAllTypeInfo()
```

### Type Info
`glas::TypeInfo` is a struct containing the type information that can normally only be accessed at compile time. the information in this struct can be customized inside of the `glas_config.h` file.

### Function Reflection
Functions and member functions can also be reflected using Glassify. using the macros `GLAS_FUNCTION(Function)` and `GLAS_MEMBER_FUNCTION(Class, MemberFunction)` for functions and member functions.

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

## Customization
The developer is able to choose which reflection data is stored inside the `glas::TypeInfo` struct by modyfing the contents of the `glas_config.h` file.

Inside of the config file the developer may add any variable that he might need for his program inside of the TypeInfo struct.

The newly added variable should then be filled inside of the `TypeInfo::Create()` function below the struct definition.

There are addons that can be toggled on or off by commenting out the `#define` for each addon

## Addons
Addons are useful features that come with Glassify that can be completely removed from the project if the developer has no need for them

### Serialization

This features allows the user to serialize a class to either JSon or Binary. The `#define GLAS_SERIALIZATION` macro must be defined inside of the `glas_config.h` file. In order to serialize member variables, the `GLAS_MEMBER` macro must be used to register the member variables into the reflection system.

```cpp
void SerializeType(std::ostream& stream, const void* data, TypeId type)
void SerializeType(std::ostream& stream, const TYPE& type)
```
These function allow the user to serialize a type to a stream in JSon format.

```cpp
void SerializeTypeBinary(std::ostream& stream, const void* data, TypeId type)
void SerializeTypeBinary(std::ostream& stream, const TYPE& type)
```
These function allow the user to serialize a type to a stream in Binary format.

```cpp
void DeserializeType(std::istream& stream, void* data, TypeId type)
void DeserializeType(std::ostream& stream, const TYPE& type)
```
These function allow the user to deserialize a type from a stream that is in JSon format. It will fill the values gotten from the stream into the instance of the type.

```cpp
void DeserializeTypeBinary(std::istream& stream, void* data, TypeId type)
void DeserializeTypeBinary(std::ostream& stream, const TYPE& type)
```
These function allow the user to deserialize a type from a stream that is in Binary format. It will fill the values gotten from the stream into the instance of the type.

### Storage

This feature allows the user to store and initialize instances of types at runtime using only the `glas::TypeId`.

#### Type Storage
The `TypeStorage` class allows for the instanciation of any class that has been added to the reflection system. The class will instanciate a given type on the Heap and is responsible for safely destroying them.

#### Type Tuple
The `TypeTuple` class works similarly to a `std::tuple` class but typeless. It stores multiple instances of types and keeps track of the types inside using an array of `VariableId`s. It can be used to call functions and can be serialized too.

