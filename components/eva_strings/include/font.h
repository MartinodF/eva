#pragma once

#define EVA_STRINGS_WIDTH 4
#define EVA_STRINGS_HEIGHT 6

struct Font {
  char letter;
  char code[EVA_STRINGS_HEIGHT][EVA_STRINGS_WIDTH];
};

// clang-format off
struct Font font[] = {
{' ', { /* Processor should ignore this */
"    ",
"    ",
"    ",
"    ",
"    ",
"    "}},
{'A', {
" ## ",
"#  #",
"#  #",
"####",
"#  #",
"#  #"}},
{'B', {
"### ",
"#  #",
"### ",
"#  #",
"#  #",
"### "}},
{'C', {
" ## ",
"#  #",
"#   ",
"#   ",
"#  #",
" ## "}},
{'D', {
"### ",
"#  #",
"#  #",
"#  #",
"#  #",
"### "}},
{'E', {
"####",
"#   ",
"### ",
"#   ",
"#   ",
"####"}},
{'F', {
"####",
"#   ",
"### ",
"#   ",
"#   ",
"#   "}},
{'G', {
" ## ",
"#  #",
"#   ",
"# ##",
"#  #",
" ###"}},
{'H', {
"#  #",
"#  #",
"####",
"#  #",
"#  #",
"#  #"}},
{'I', {
"### ",
" #  ",
" #  ",
" #  ",
" #  ",
"### "}},
{'J', {
" ###",
"   #",
"   #",
"   #",
"#  #",
" ## "}},
{'K', {
"#  #",
"# # ",
"##  ",
"# # ",
"#  #",
"#  #"}},
{'L', {
"#   ",
"#   ",
"#   ",
"#   ",
"#   ",
"####"}},
{'M', {
"#  #",
"####",
"#  #",
"#  #",
"#  #",
"#  #"}},
{'N', {
"#  #",
"#  #",
"## #",
"# ##",
"#  #",
"#  #"}},
{'O', {
" ## ",
"#  #",
"#  #",
"#  #",
"#  #",
" ## "}},
{'P', {
"### ",
"#  #",
"#  #",
"### ",
"#   ",
"#   "}},
{'Q', {
" ## ",
"#  #",
"#  #",
"#  #",
"# # ",
" # #"}},
{'R', {
"### ",
"#  #",
"#  #",
"### ",
"#  #",
"#  #"}},
{'S', {
" ###",
"#   ",
" ## ",
"   #",
"   #",
"### "}},
{'T', {
"####",
" #  ",
" #  ",
" #  ",
" #  ",
" #  "}},
{'U', {
"#  #",
"#  #",
"#  #",
"#  #",
"#  #",
" ## "}},
{'V', {
"#  #",
"#  #",
"#  #",
"#  #",
" # #",
"  # "}},
{'W', {
"#  #",
"#  #",
"#  #",
"#  #",
"####",
"#  #"}},
{'X', {
"#  #",
"#  #",
" ## ",
"#  #",
"#  #",
"#  #"}},
{'Y', {
"#  #",
"#  #",
" # #",
"  # ",
"  # ",
"  # "}},
{'Z', {
"####",
"  # ",
" #  ",
" #  ",
"#   ",
"####"}},
{'0', {
" ## ",
"#  #",
"# ##",
"## #",
"#  #",
" ## "}},
{'1', {
" #  ",
"##  ",
" #  ",
" #  ",
" #  ",
"### "}},
{'2', {
" ## ",
"#  #",
"   #",
" ## ",
"#   ",
"####"}},
{'3', {
" ## ",
"#  #",
"  # ",
"   #",
"#  #",
" ## "}},
{'4', {
"  # ",
" ## ",
"# # ",
"####",
"  # ",
"  # "}},
{'5', {
"####",
"#   ",
"### ",
"   #",
"#  #",
" ## "}},
{'6', {
" ## ",
"#   ",
"### ",
"#  #",
"#  #",
" ## "}},
{'7', {
"####",
"   #",
"  # ",
" #  ",
" #  ",
" #  "}},
{'8', {
" ## ",
"#  #",
" ## ",
"#  #",
"#  #",
" ## "}},
{'9', {
" ## ",
"#  #",
"#  #",
" ###",
"   #",
" ## "}},
{'!', {
" #  ",
" #  ",
" #  ",
" #  ",
"    ",
" #  "}},
{'.', {
"    ",
"    ",
"    ",
"    ",
"    ",
" #  "}},
{',', {
"    ",
"    ",
"    ",
"    ",
" #  ",
"#   "}},
{'?', {
" ## ",
"#  #",
"  # ",
" #  ",
"    ",
" #  "}},
{'%', {
"#  #",
"  ##",
"  # ",
" #  ",
"##  ",
"#  #"}},
{'#', {
" # #",
"## #",
" ###",
"### ",
"# ##",
"# # "}},
{'_', {
"    ",
"    ",
"    ",
"    ",
"    ",
"####"}},
{'-', {
"    ",
"    ",
"####",
"    ",
"    ",
"    "}},
{';', {
"    ",
"    ",
" #  ",
"    ",
" #  ",
"#   "}},
{'`', {
"#   ",
"#   ",
"    ",
"    ",
"    ",
"    "}},
{'=', {
"    ",
"####",
"    ",
"####",
"    ",
"    "}},
{':', {
"    ",
"    ",
"    ",
" #  ",
"    ",
" #  "}},
{'<', {
"  # ",
" #  ",
"#   ",
" #  ",
"  # ",
"    "}},
{'>', {
" #  ",
"  # ",
"   #",
"  # ",
" #  ",
"    "}},
{'~', {
"    ",
"    ",
" # #",
"# # ",
"    ",
"    "}},
{'*', {
" ## ",
" ## ",
"####",
" ## ",
"#  #",
"#  #"}},
{'/', {
"    ",
"   #",
"  # ",
" #  ",
"#   ",
"    "}},
{'\'', {
"    ",
"#   ",
" #  ",
"  # ",
"   #",
"    "}},
{'"', {
"# # ",
"# # ",
"    ",
"    ",
"    ",
"    "}},
{'(', {
"  # ",
" #  ",
" #  ",
" #  ",
"  # ",
"    "}},
{')', {
" #  ",
"  # ",
"  # ",
"  # ",
" #  ",
"    "}},
{'}', {
"### ",
"  # ",
"  ##",
"  # ",
"### ",
"    "}},
{'{', {
" ###",
" #  ",
"##  ",
" #  ",
" ###",
"    "}},
{'+', {
"    ",
" #  ",
"### ",
" #  ",
"    ",
"    "}},
{'[', {
" ###",
" #  ",
" #  ",
" #  ",
" ###",
"    "}},
{']', {
"### ",
"  # ",
"  # ",
"  # ",
"### ",
"    "}},
{0, {
"####",
"####",
"####",
"####",
"####",
"####"}},
};
// clang-format on
