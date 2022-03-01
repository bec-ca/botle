#include <iostream>

#include "evaluate.hpp"
#include "suggest.hpp"
#include "utils/command.hpp"
#include "word_counter.hpp"

int main(int argc, char* argv[])
{
  return CommandGroupBuilder()
    .cmd("suggest", Suggest::command())
    .cmd("evaluate", Evaluate::command())
    .cmd("count-word", WordCounter::command())
    .build()
    .run(argc, argv);
}
