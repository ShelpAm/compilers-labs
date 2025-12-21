#include <cassert>
#include <determinstic-finite-automaton.h>
#include <print>

int main()
{
    auto nfa = NFA({0}, {7});
    nfa.add(0, 1, 'b');
    nfa.add(1, 2, NFA::epsilon);
    nfa.add(2, 3, 'a');
    nfa.add(3, 2, 'b');
    nfa.add(2, 5, NFA::epsilon);
    nfa.add(5, 6, 'a');
    nfa.add(6, 7, 'b');
    nfa.add(1, 4, 'b');
    nfa.add(4, 5, 'b');
    nfa.add(5, 1, NFA::epsilon);
    auto converted = DFA::from_nfa(nfa);
    auto hand = DFA(0, {4});
    hand.add(0, 1, 'b');
    hand.add(1, 2, 'a');
    hand.add(1, 4, 'b');
    hand.add(2, 4, 'b');
    hand.add(4, 2, 'a');
    hand.add(4, 0, 'b');

    std::string text{"babab"};
    // std::print("Input test text: ");
    // std::cin >> text;
    std::println("Using text: {}", text);
    auto converted_match = converted.match(text);
    auto hand_match = hand.match(text);
    if (converted_match == hand_match) {
        std::println(
            "The matching result for \"{}\" is the same for two dfas: {}.",
            text, converted_match ? "matched" : "unmatched");
    }
    else {
        std::println("The matching result for \"{}\" is different for two "
                     "dfas. Converted "
                     "{}, but hand {}.",
                     text, converted_match ? "matched" : "unmatched",
                     hand_match ? "matched" : "unmatched");
    }

    // Simple test from zzs
    {
        auto fa = DFA(0, {3});
        fa.add(0, 1, 'a');
        fa.add(1, 3, 'c');
        fa.add(0, 2, 'b');
        fa.add(2, 3, 'a');
        fa.add(3, 2, 'b');
        assert(fa.match("acbabababa"));
        assert(!fa.match("acbababab"));
    }
}
