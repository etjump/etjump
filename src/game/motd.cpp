#include "motd.hpp"
#include "../json/json.h"
#include "g_local.hpp"
#include "g_utilities.hpp"
#include <fstream>

Motd::Motd() : initialized_(false)
{
}

Motd::~Motd()
{
}

void Motd::Initialize()
{
    if (strlen(g_motdFile.string) == 0)
    {
        return;
    }

    std::string filePath = GetPath(g_motdFile.string);
    std::ifstream f(filePath.c_str());

    if (!f)
    {
        G_LogPrintf("MOTD: g_motdFile was defined but couldn't find the motd file. To generate an example file do /rcon generateMotd <file name>.\n");
        return;
    }

    std::string content((std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>());

    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(content, root))
    {
        G_LogPrintf("MOTD: failed to parse motd file: %s\n", reader.getFormattedErrorMessages().c_str());
        return;
    }

    try
    {
        chatMessage_= root["chat_message"].asString();
        motd_ = root["console_message"].asString();
    }
    catch (...)
    {
        G_LogPrintf("MOTD: missing information from motd file. Are you sure both chat_message and console_message are defined?\n");
    }

    G_LogPrintf("MOTD: initialized.\n");

    initialized_ = true;
}

void Motd::PrintMotd(gentity_t* ent)
{
    
    ChatPrintTo(ent, chatMessage_);
    ConsolePrintTo(ent, motd_);
}

void Motd::GenerateMotdFile()
{
    Json::Value root;
    root["chat_message"] = "This is the chat message.";
    root["console_message"] = "This is the console message.";
    Json::StyledWriter writer;
    std::string output = writer.write(root);
    std::ofstream fOut(GetPath(g_motdFile.string).c_str());
    if (!fOut)
    {
        G_Printf("Couldn't open file \"%s\" defined in g_motdFile.\n", g_motdFile.string);
        return;
    }
    fOut << output;
    fOut.close();
    G_Printf("Generated new motd file \"%s\"\n", g_motdFile.string);
}
