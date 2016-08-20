#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <boost/format.hpp>
#include "etj_client_print_utilities.h"
#include "etj_client_commands_handler.h"
#include "etj_trickjump_lines.h"
#include "etj_client.h"
#include "../game/etj_file.h"
#include <set>
#include "../json/json.h"

ETJump::TrickjumpLines::TrickjumpLines(ClientCommandsHandler* clientCommandsHandler, ClientCommandsHandler* serverCommandsHandler, const std::string& currentMap)
	:_clientCommandsHandler(clientCommandsHandler), _serverCommandsHandler(serverCommandsHandler), _tjlDirectory("tjlines/" + currentMap)
{
	if (!_clientCommandsHandler)
	{
		Client::error("TrickjumpLines::clientCommandHandler is null.");
		return;
	}

	if (!_serverCommandsHandler)
	{
		Client::error("TrickjumpLines::serverCommandsHandler is null.");
		return;
	}

	subscribeToClientCommands();
	subscribeToServerCommands();
}

ETJump::TrickjumpLines::~TrickjumpLines()
{
	unsubscribeFromClientCommands();
	unsubscribeFromClientCommands();
}

void ETJump::TrickjumpLines::subscribeToClientCommands()
{
	const auto listNamespaces = "tjl_listNamespaces";
	Client::addCommand(listNamespaces);
	_clientCommandsHandler->subscribe(listNamespaces, [&](const std::vector<std::string>& args)
	{
		listFiles();
	});

	const auto loadNamespacesText = "tjl_loadNamespaces";
	Client::addCommand(loadNamespacesText);
	_clientCommandsHandler->subscribe(loadNamespacesText, [&](const std::vector<std::string>& args)
	{
		loadNamespaces(args);
	});

	_clientCommandsHandler->subscribe("listRoutes", [&](const std::vector<std::string>& args)
	{

	});
}

void ETJump::TrickjumpLines::unsubscribeFromClientCommands()
{
}

void ETJump::TrickjumpLines::subscribeToServerCommands()
{
}

void ETJump::TrickjumpLines::unsubscribeFromServerCommands()
{
}

void ETJump::TrickjumpLines::listFiles() const
{
	std::vector<std::string> matches;
	try
	{
		matches = File::localFileList(_tjlDirectory, ".json");
	}
	catch (File::PathNotFoundException ex)
	{
		ClientPrintUtilities::printToConsole(boost::format("^3Note: ^7There are currently no trickjump line definition files in %s directory.\n") % _tjlDirectory);
		return;
	}
	catch (std::runtime_error ex)
	{
		ClientPrintUtilities::printToConsole("^1Error: ^7Unknown error occurred while trying to list trickjump lines definition files.\n");
		return;
	}

	for (auto& m : matches)
	{
		m = m.substr(0, m.find_last_of("."));
	}
	sort(begin(matches), end(matches));

	ClientPrintUtilities::printToConsole(boost::format(
		"^zFound %d possible trickjump line namespace(s)\n"
		"^7---------------------------------------------\n"
		"%s\n"
	) % matches.size() % boost::join(matches, "\n"));
}

std::vector<std::string> ETJump::TrickjumpLines::getSortedNamespaces()
{
	// std::set seems to be fairly efficient
	// http://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
	auto pk3Namespaces = File::fileList(_tjlDirectory, ".json");
	auto localNamespaces = File::localFileList(_tjlDirectory, ".json");
	pk3Namespaces.insert(end(pk3Namespaces), begin(localNamespaces), end(localNamespaces));
	std::set<std::string> namespacesSet(begin(pk3Namespaces), end(pk3Namespaces));

	std::vector<std::string> namespaces(begin(namespacesSet), end(namespacesSet));
	for (auto& ns : namespaces)	ns = ns.substr(0, ns.find_last_of("."));
	return namespaces;
}

void ETJump::TrickjumpLines::loadNamespace(const std::string& ns)
{
	try
	{
		auto file = File((boost::format("%s/%s.json") % _tjlDirectory % ns).str(), File::Mode::Read);
		auto contents = file.read();
		std::string json(begin(contents), end(contents));

		Json::Value root;
		Json::Reader reader;

		if (!reader.parse(json, root))
		{
			ClientPrintUtilities::printToConsole(boost::format("^1Error: ^7could not parse namespace \"%s\": %s\n") % ns % reader.getFormattedErrorMessages());
			return;
		}

		std::vector<Route> routes;
		try
		{
			for (auto & prop : root)
			{
				Route route;

				// route props
				route.name = prop["name"].asString();
				route.width = prop["width"].asFloat();

				// route color
				auto color = prop["color"];
				for (int i = 0, len = color.size(); i < len; ++i)
				{
					route.color[i] = color.asUInt();
				}

				// route trails
				auto jsonTrails = prop["trails"];
				std::vector<std::vector<Node>> trails;

				for (const auto & jsonTrail : jsonTrails)
				{
					std::vector<Node> trail;
					for (const auto & jsonNode : jsonTrail)
					{
						Node node;
						auto coordinates = jsonNode["coordinates"];
						for (int i = 0, len = coordinates.size(); i < len; ++i)
						{
							node.coordinates[i] = coordinates[i].asFloat();
						}
						trail.push_back(node);
					}
				}
				route.trails = trails;
				routes.push_back(route);
			}
		} catch (const std::runtime_error& e)
		{
			ClientPrintUtilities::printToConsole(boost::format("^1Error: ^7invalid route data: %s\n") % e.what());
		}

		_routes[ns] = move(routes);
	} catch (File::FileNotFoundException ex)
	{
		// Should never happen. We check for existing namespaces before getting this far
		ClientPrintUtilities::printToConsole(boost::format("^1Error: ^7could not find file: %s\n") % (ns + ".json"));
	}
}

void ETJump::TrickjumpLines::loadNamespaces(const std::vector<std::string>& args)
{
	try
	{
		auto namespaces = getSortedNamespaces();

		std::vector<std::string> notFound;

		bool skip = true;
		for (auto arg : args)
		{
			if (skip)
			{
				skip = false;
				continue;
			}
			if (boost::ends_with(arg, ".json"))
			{
				arg = arg.substr(0, arg.size() - 5);
			}
			
			auto match = find(begin(namespaces), end(namespaces), arg);
			if (match == end(namespaces))
			{
				notFound.push_back(arg);
				continue;
			}

			loadNamespace(arg);
		}

		if (notFound.size())
		{
			ClientPrintUtilities::printToConsole(boost::format(
				"^3Error: ^7could not find \"%s\" namespace(s). Available namespaces include:\n"
				"* %s\n"
			) % boost::join(notFound, ", ") % boost::join(namespaces, "\n"));
		}
	}
	catch (File::PathNotFoundException ex)
	{
		ClientPrintUtilities::printToConsole(boost::format("^3Error: ^7There are currently no trickjump line definition files in %s directory.\n") % _tjlDirectory);
		return;
	}
	catch (std::runtime_error ex)
	{
		ClientPrintUtilities::printToConsole("^1Error: ^7Unknown error occurred while trying to list trickjump lines definition files.\n");
		return;
	}
	
}
