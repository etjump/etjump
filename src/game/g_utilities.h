#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <string>

extern "C" {
#include "g_local.h"
}

using std::vector;
using std::string;

vector<string> GetArgs();
std::string G_SHA1(const std::string& str);
string int2string(int value);
bool string2int(const string& s, int& i);
void LogPrint(string msg);
void LogPrintln(string msg);
gentity_t *playerFromName(const string& name, string& error);
void ChatPrintTo(gentity_t *ent, const string& message);
void ChatPrintAll(const string& message);
void CPMPrintTo(gentity_t *ent, const string& message);
void CPMPrintAll(const string& message);
void CPPrintTo(gentity_t *ent, const string& message);
void CPPrintAll(const string& message);
void PrintTo(gentity_t *ent, const string& message);
void PrintAll(const string& message);
vector<string> GetSayArgs();
void beginBufferPrint();
void finishBufferPrint(gentity_t *ent);
void bufferPrint(gentity_t *ent, const string& msg);
#endif