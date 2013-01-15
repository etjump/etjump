#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <string>

extern "C" {
#include "g_local.h"
}

using std::vector;
using std::string;

const string SEPARATOR = " ";

string RemoveColors(const string& source);
void SanitizeString(const string& in, string& out, bool to_lower);

void ChatPrintTo(gentity_t *ent, const string& message);
void CPMPrintTo(gentity_t *ent, const string& message);
void CPPrintTo(gentity_t *ent, const string& message);
void PrintTo(gentity_t *ent, const string& message);

void BannerPrintAll(const string& message);
void ChatPrintAll(const string& message);
void CPMPrintAll(const string& message);
void CPPrintAll(const string& message);
void PrintAll(const string& message);

void LogPrintln(const string& message);
void LogPrint(const string& message);

void BeginBufferPrint();
void FinishBufferPrint(gentity_t *ent);
void BufferPrint(gentity_t *ent, const string& message);

vector<string> GetSayArgs();
vector<string> GetArgs();

string IntToString(int to_convert);
bool StringToInt(const string& source, int& target);

string SHA1(const string& to_hash);
gentity_t *PlayerForName(const string& name, string& error);

// Doesn't really belong here.
// Located at g_maplist.cpp
const vector<string> *G_GetMapList();

#endif