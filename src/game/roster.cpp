#ifdef __linux__
#include <jsoncpp/json.h>
#else
#include <json/json.h>
#endif
#include <assert.h>

#include <boost/foreach.hpp>

#include <fstream>

#include "roster.h"

#include "fs.h"

Roster::Roster(std::istream &input_stream)
{
    // parse the input_stream as a JSON document
    Json::Value doc;
    Json::Reader reader;
    bool success;

    success = reader.parse(input_stream, doc);
    assert(success);

    // the document should look like:
    // [
    //   { player: 0, groups: [ /* group */, /* group */, /* group */ ] },
    //   { player: 1, groups: [ /* group */, /* group */, /* group */ ] }
    // ]

    assert(doc.isArray());

    for (int i = 0; i < doc.size(); i++) {
        Json::Value &player_object = doc[i];
        assert(player_object.isObject());

        int player_number = player_object.get("player", -1).asInt();
        assert(player_number == 0 || player_number == 1);

        // walk the groups array
        Json::Value &groups = player_object["groups"];
        assert(groups.isArray());

        for (int j = 0; j < groups.size(); j++) {
            // parse each array of astronauts into a RosterGroup
            Json::Value &group_array = groups[j];
            RosterGroup group(player_number, j + 1, group_array);

            // hang onto this group
            m_groups.push_back(group);
        }
    }
}

Roster::~Roster()
{
}

RosterGroup &Roster::getGroup(int player, int group_number)
{
    BOOST_FOREACH(RosterGroup & roster_group, m_groups) {
        if (roster_group.getPlayer() == player && roster_group.getGroupNumber() == group_number) {
            return roster_group;
        }
    }

    // FIXME: this shouldn't happen unless there's a data problem
    // still... should probably trap this better
    assert(false);
}

Roster Roster::load(const std::string filename_str)
{
    char *filename = locate_file(filename_str.c_str(), FT_DATA);
    assert(filename);

    std::ifstream roster_file(filename);
    Roster roster(roster_file);

    free(filename);

    return roster;
}
