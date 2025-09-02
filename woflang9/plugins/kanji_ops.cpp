// ==================================================
// FIXED: kanji_ops.cpp
// ==================================================
#include "core/woflang.hpp"
#include <iostream>

extern "C" {

struct KanjiEntry {
    const char* kanji;
    const char* kana;
    const char* romaji;
    const char* meaning;
    const char* sample;
};

static const KanjiEntry basic_kanji[] = {
    {"日", "にち", "nichi", "day; sun", "今日はいい日ですね (kyou wa ii hi desu ne)"},
    {"本", "ほん", "hon", "book; origin; Japan", "日本 (nihon) = Japan"},
    {"人", "ひと", "hito", "person", "日本人 (nihonjin) = Japanese person"},
    {"大", "だい", "dai", "big", "大学 (daigaku) = university"},
    {"小", "しょう", "shou", "small", "小学生 (shougakusei) = elementary student"},
    {"学", "がく", "gaku", "study; learning", "学生 (gakusei) = student"},
    {"先", "せん", "sen", "ahead; previous", "先生 (sensei) = teacher"},
    {"生", "せい", "sei", "life; birth", "誕生日 (tanjoubi) = birthday"},
    {"友", "とも", "tomo", "friend", "友達 (tomodachi) = friend"},
    {"時", "じ", "ji", "time; hour", "今何時？(ima nanji?) = What time is it?"},
    {"今", "いま", "ima", "now", "今すぐ (ima sugu) = right now"},
    {"私", "わたし", "watashi", "I; me", "私は学生です (watashi wa gakusei desu)"},
    {"何", "なに", "nani", "what", "何ですか？ (nan desu ka)"},
    {"行", "い", "i/iku", "go", "行きます (ikimasu) = go"},
    {"見", "み", "mi", "see; look", "見て！(mite!) = Look!"},
    {"聞", "き", "ki", "hear; ask", "聞いて (kiite) = Listen/Ask"},
    {"食", "た", "ta", "eat", "食べます (tabemasu) = eat"},
    {"飲", "の", "no", "drink", "飲みます (nomimasu) = drink"},
    {"買", "か", "ka", "buy", "買い物 (kaimono) = shopping"},
    {"安", "やす", "yasu", "cheap; safe", "安い (yasui) = cheap"},
    {"高", "たか", "taka", "tall; expensive", "高い (takai) = expensive"},
    {"新", "しん", "shin", "new", "新聞 (shinbun) = newspaper"},
    {"古", "ふる", "furu", "old", "古い (furui) = old (thing)"},
    {"電", "でん", "den", "electricity", "電話 (denwa) = telephone"},
    {"話", "はな", "hana", "talk; story", "話します (hanashimasu) = speak"},
    {"車", "くるま", "kuruma", "car", "車で行きます (kuruma de ikimasu)"},
    {"駅", "えき", "eki", "station", "駅前 (ekimae) = in front of the station"},
    {"家", "いえ", "ie", "house", "家に帰ります (ie ni kaerimasu)"},
    {"道", "みち", "michi", "road; way", "道を歩く (michi o aruku)"},
};

static const KanjiEntry basic_hiragana[] = {
    {"あ", "あ", "a", "hiragana: a", "ありがとう (arigatou) = thanks"},
    {"い", "い", "i", "hiragana: i", "いいね (ii ne) = good, nice"},
    {"う", "う", "u", "hiragana: u", "うまい (umai) = tasty"},
    {"え", "え", "e", "hiragana: e", "ええと (eeto) = um..."},
    {"お", "お", "o", "hiragana: o", "おはよう (ohayou) = good morning"},
    {"ん", "ん", "n", "hiragana: n", "うん (un) = yes"},
};

static const KanjiEntry basic_katakana[] = {
    {"ア", "あ", "a", "katakana: a", "アイス (aisu) = ice (ice cream)"},
    {"イ", "い", "i", "katakana: i", "イギリス (igirisu) = UK"},
    {"ウ", "う", "u", "katakana: u", "ウエイト (ueito) = weight"},
    {"エ", "え", "e", "katakana: e", "エコ (eko) = eco"},
    {"オ", "お", "o", "katakana: o", "オレンジ (orenji) = orange"},
};

static const KanjiEntry slang[] = {
    {"www", "わら", "wara", "lol (slang)", "めっちゃwww (meccha wara) = so funny"},
    {"乙", "おつ", "otsu", "good job/thanks for work", "乙！(otsu!) = gg, nice work"},
    {"神", "かみ", "kami", "godly, awesome", "神回 (kamikai) = god-tier episode"},
    {"草", "くさ", "kusa", "lol (from 'wwww')", "草生える (kusa haeru) = that's hilarious"},
    {"リア充", "りあじゅう", "riajuu", "normie, happy IRL", "リア充爆発しろ (riajuu bakuhatsu shiro) = normies go explode!"},
    {"ガチ", "がち", "gachi", "seriously, for real", "ガチでやばい (gachi de yabai) = that's really crazy"},
};

static const KanjiEntry kaomoji[] = {
    {"(・ω・)", "(・ω・)", "(・ω・)", "cute neutral face", "こんにちは (konnichiwa) (・ω・)"},
    {"＼(^o^)／", "(banzai)", "banzai", "yay!", "やったー！＼(^o^)／"},
    {"(T_T)", "(tears)", "tears", "sad", "悲しい (kanashii) (T_T)"},
    {"(^_−)−☆", "wink", "wink", "wink, good job", "ナイス！(^_−)−☆"},
};

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    // Core kanji ops
    for (const auto& entry : basic_kanji) {
        (*op_table)[entry.kanji] = [entry](std::stack<woflang::WofValue>&) {
            std::cout << entry.kanji << " [" << entry.kana << "] (" << entry.romaji << ")"
                      << " - " << entry.meaning << "\n"
                      << "Example: " << entry.sample << "\n";
        };
    }
    
    // Hiragana ops
    for (const auto& entry : basic_hiragana) {
        (*op_table)[entry.kanji] = [entry](std::stack<woflang::WofValue>&) {
            std::cout << entry.kanji << " [" << entry.kana << "] (" << entry.romaji << ")"
                      << " - " << entry.meaning << "\n"
                      << "Example: " << entry.sample << "\n";
        };
    }
    
    // Katakana ops
    for (const auto& entry : basic_katakana) {
        (*op_table)[entry.kanji] = [entry](std::stack<woflang::WofValue>&) {
            std::cout << entry.kanji << " [" << entry.kana << "] (" << entry.romaji << ")"
                      << " - " << entry.meaning << "\n"
                      << "Example: " << entry.sample << "\n";
        };
    }
    
    // Slang/abbreviations
    for (const auto& entry : slang) {
        (*op_table)[entry.kanji] = [entry](std::stack<woflang::WofValue>&) {
            std::cout << entry.kanji << " [" << entry.kana << "] (" << entry.romaji << ")"
                      << " - " << entry.meaning << "\n"
                      << "Example: " << entry.sample << "\n";
        };
    }
    
    // Kaomoji/emojis
    for (const auto& entry : kaomoji) {
        (*op_table)[entry.kanji] = [entry](std::stack<woflang::WofValue>&) {
            std::cout << entry.kanji << " : " << entry.meaning << "\n"
                      << "Sample: " << entry.sample << "\n";
        };
    }
    
    // Educational operations
    (*op_table)["kanji_info"] = [](std::stack<woflang::WofValue>&) {
        std::cout << "=== Kanji & Japanese Text Information ===\n\n";
        std::cout << "Japanese uses three writing systems:\n";
        std::cout << "1. Hiragana (ひらがな) - Phonetic script for Japanese words\n";
        std::cout << "2. Katakana (カタカナ) - Phonetic script for foreign words\n";
        std::cout << "3. Kanji (漢字) - Chinese characters for meaning\n\n";
        std::cout << "Woflang includes basic kanji, hiragana, katakana, and internet slang!\n";
        std::cout << "Try typing any Japanese character to see its information.\n";
    };
}
