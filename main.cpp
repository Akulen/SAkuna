#include <bits/stdc++.h>

#include "sakuna.hpp"
#include "uci.hpp"

int main() {
    uci uci;
    SAkuna engine = SAkuna(uci);

    // Register callbacks to the messages from the UI and respond appropriately.
    uci.receive_uci.connect([&] ()
    {
        uci.send_id("Sakuna 0.0", "Akulen");
        //uci.send_option_uci_limit_strength(false);
        uci.send_uci_ok();
    });
    uci.receive_is_ready.connect([&] ()
    {
        engine.init();    
        uci.send_ready_ok();
    });
    uci.receive_position.connect([&] (const std::string& fen, const std::vector<std::string>& moves)
    {
        engine.set_position(fen, moves);
    });
    uci.receive_go.connect([&] (const std::map<uci::command, std::string>& commands)
    {
        if(commands.count(uci::command::perft)) {
            std::string depth = commands.at(uci::command::perft);
            engine.divide(atoi(depth.c_str()));
        } else {
            int wtime = commands.count(uci::command::white_time) ?
                atoi(commands.at(uci::command::white_time).c_str()) : 100'000;
            int btime = commands.count(uci::command::black_time) ?
                atoi(commands.at(uci::command::black_time).c_str()) : 100'000;
            engine.start_search(wtime, btime);
        }
    });

    // Start communication with the UI through console.
    uci.launch();

    return 0;
}
