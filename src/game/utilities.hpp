#ifndef g_utilities_h__
#define g_utilities_h__

#include <string>
#include <vector>
#include "etj_local.h"

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

// Argument handling
typedef const std::vector<std::string> *Arguments;
typedef std::vector<std::string>::const_iterator ConstArgIter;
typedef std::vector<std::string>::iterator ArgIter;
Arguments GetArgs();
Arguments GetSayArgs(int start = 0);
// returns an empty string if not so many args
std::string SayArgv(int arg);

// Conversions
bool ToInt(const std::string &toConvert, int &value);
bool ToUnsigned(const std::string &toConvert, unsigned &value);
bool ToFloat(const std::string &toConvert, float &value);

std::string ToString(vec3_t toConvert);
std::string ToString(vec_t x, vec_t y, vec_t z);

gentity_t *PlayerGentityFromString(char *name, char *err, int size,
                                   team_t filter = TEAM_FREE);
gentity_t *PlayerGentityFromString(const std::string &name, std::string &err,
                                   team_t filter = TEAM_FREE);

std::string ValueForKey(gentity_t *ent, const std::string &key);
std::string ValueForKey(int clientNum, const std::string &key);
std::string TimeStampToString(int64_t timeStamp);
// TODO: this should be part of 'Time' struct (needs 'operator-' implementation)
std::string TimeStampDifferenceToString(int diff);
#endif // g_utilities_h__
