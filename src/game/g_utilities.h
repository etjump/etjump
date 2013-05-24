#ifndef g_utilities_h__
#define g_utilities_h__

#include "g_local.hpp"

#ifdef max
#undef max
#endif

#include <string>
#include <vector>

const std::string NEWLINE = "\n";

// C++ versions of the printing functions
void BPAll( const std::string& msg );
void BPTo( gentity_t *target, const std::string& msg );
void BeginBufferPrint();
void BufferPrint( gentity_t *ent, const std::string& msg );
void FinishBufferPrint(gentity_t *ent, bool insertNewLine=false);
void CPAll( const std::string& msg );
void CPMAll( const std::string& msg );
void CPMTo( gentity_t *target, const std::string& msg );
void CPTo( gentity_t *target, const std::string& msg );
void ChatPrintAll( const std::string& msg );
void ChatPrintTo( gentity_t *target, const std::string& msg );
void ConsolePrintAll( const std::string& msg );
void ConsolePrintTo( gentity_t *target, const std::string& msg );

// Argument handling
typedef const std::vector<std::string> *Arguments;
typedef std::vector<std::string>::const_iterator ConstArgIter;
typedef std::vector<std::string>::iterator ArgIter;
Arguments GetArgs();
Arguments GetSayArgs();

// Conversions
bool StringToInt(const std::string& toConvert, int& value);
std::string IntToString( int value );

std::string Vec3ToString( vec3_t toConvert );
std::string Vec3ToString( vec_t x, vec_t y, vec_t z );

gentity_t *PlayerGentityFromString(char *name, char *err, int size);
gentity_t *PlayerGentityFromString(const std::string& name, 
                                   char *err, int size);
// Removes duplicate chars
void RemoveDuplicates(std::string& out);
void CharPtrToString( const char* p, std::string& s );
#endif // g_utilities_h__
