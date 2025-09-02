WofLang Easter Egg Plugins 🥚⚡

Overview

WofLang includes two delightful easter egg plugins that add mystical chaos and humor to your mathematical explorations. These hidden gems activate through specific commands and provide moments of levity during intense computational sessions.



Moses Riddle Plugin (moses\_op.cpp)

Description

A mystical riddle plugin that transforms your WofLang session into a Hebrew mysticism experience. Features probabilistic activation, UTF-8 Hebrew text support, and philosophical contemplation.

Commands

那 (Chinese: "that")



Behavior: Has a ~1% chance to trigger the Moses riddle

Effect: Transforms the session into Hebrew mode

Visual: Displays riddle in both English and Hebrew

State: Once activated, remains active for the entire session



answer



Prerequisite: Hebrew mode must be active

Effect: Reveals the answer to Moses' tea riddle

Output: "He brews it" in English and Hebrew



reset



Effect: Returns the session to normal mode

Output: Confirms world has returned to former shape



Technical Features



UTF-8 Console Setup: Automatically configures console for Hebrew display

Cross-Platform: Works on Windows (with CP\_UTF8) and Unix systems

Persistent State: Hebrew mode survives until manual reset

Thread-Safe: Uses proper static initialization



Example Usage

woflangwof> 那

\# (1% chance) 

\# 那... How does Moses make his tea?

\# 

\# ...העולם השתנה

\# (The world has changed... type 'answer' to respond)



wof> answer

\# He brews it.

\# הוא מכין תה... (He brews it.)



wof> reset

\# The world returns to its former shape.

Philosophy

Includes the famous quote from Hillel the Elder:



"If I am not for myself, who will be for me? And when I am for myself, what am 'I'? And if not now, when?"





Prime Heck Plugin (prime\_heck\_op.cpp)

Description

A chaotic typo-activated easter egg that summons ASCII art demons and performs stack destruction. Perfect for when you accidentally type pime\_heck instead of prime\_check.

Commands

pime\_heck



Trigger: Common typo of prime\_check

Effect: Summons ASCII art creature

Destruction: Completely clears the stack

Philosophy: Embraces chaos and computational accidents



Visual Output

&nbsp;         .--.

&nbsp;        |o\_o |

&nbsp;        |:\_/ |

&nbsp;       //   \\ \\

&nbsp;      (|     | )

&nbsp;     /'\\\_   \_/`\\

&nbsp;     \\\_\_\_)=(\_\_\_/



A typo, or a summons from the deep?

You typed 'pime\_heck' instead of 'prime\_check'.

Example Usage

woflangwof> 42 17 23 prime\_check  # Oops, typo!

wof> 42 17 23 pime\_heck



\# \[ASCII art appears]

\# The stack has been dragged to heck.



wof> .s

\# Stack: \[]  (completely empty!)



Installation Instructions

1\. Build Setup

Add to your CMakeLists.txt:

cmake# Moses Plugin

add\_library(moses\_op SHARED plugins/moses\_op.cpp)

target\_link\_libraries(moses\_op woflang\_core)

if(WIN32)

&nbsp;   target\_compile\_options(moses\_op PRIVATE /utf-8)

endif()



\# Prime Heck Plugin

add\_library(prime\_heck\_op SHARED plugins/prime\_heck\_op.cpp)

target\_link\_libraries(prime\_heck\_op woflang\_core)

2\. File Placement

woflang/

├── plugins/

│   ├── moses\_op.cpp        # Moses riddle source

│   └── prime\_heck\_op.cpp   # Prime heck source

├── build/plugins/

│   ├── moses\_op.dll/.so    # Compiled Moses plugin

│   └── prime\_heck\_op.dll/.so # Compiled prime heck plugin

3\. Build Commands

bashcd build

cmake --build . --target moses\_op

cmake --build . --target prime\_heck\_op

4\. Testing

bash# Test Moses riddle

./woflang -c "那 answer reset"



\# Test prime heck

./woflang -c "42 17 23 pime\_heck"



Technical Notes

Unicode Support



Moses Plugin: Requires UTF-8 console support for Hebrew text

Windows: Automatically sets CP\_UTF8 console code page

Linux/macOS: Uses system UTF-8 locale settings

Terminal: Must support RTL (Right-to-Left) rendering for proper Hebrew display



Memory Management



Stack Safety: Both plugins properly handle stack operations

Static State: Moses plugin uses static variables for persistent Hebrew mode

Thread Safety: Proper initialization guards for multi-threaded environments



Integration Points



Plugin Loading: Standard WofLang plugin interface

Operation Registration: Uses OpTable for command registration

Error Handling: Graceful degradation if UTF-8 support unavailable





Design Philosophy

These easter eggs embody the playful, mystical spirit of WofLang:



Serendipity: Moses riddle activates probabilistically, creating moments of surprise

Cultural Depth: Hebrew text and philosophical quotes add intellectual richness

Error Tolerance: Prime heck transforms typos into features rather than failures

Chaos Mathematics: Stack destruction as a form of computational performance art



The Deeper Meaning

Just as Moses brought tablets down from the mountain, these plugins bring moments of transcendence to mathematical computation. The prime heck plugin reminds us that in the realm of symbolic computation, even our mistakes can become gateways to unexpected beauty.



Troubleshooting

Hebrew Text Not Displaying



Check console UTF-8 support

Verify terminal font includes Hebrew glyphs

On Windows, ensure proper code page setting

Test with simple Hebrew: echo "שלום"



Plugin Not Loading



Verify plugin files in correct directory

Check CMake build output for errors

Ensure woflang\_core linkage

Test plugin registration with .ops command



Stack Behavior



Moses plugin doesn't affect stack contents

Prime heck plugin intentionally clears entire stack

Use reset to return Moses plugin to normal state





Future Enhancements

Potential expansions for these easter eggs:



Moses Extended: Additional biblical computational parables

Multiple Languages: Expand Unicode support to Sanskrit, Arabic, etc.

Interactive Riddles: Multi-step puzzle sequences

ASCII Art Gallery: Collection of mathematical creature summons

Probability Tuning: User-configurable trigger rates





"In the beginning was the Word, and the Word was with Math, and the Word was Math."

— The Gospel According to Stack-Based Computing

🐺⚡ Happy computing, husklyfren!

