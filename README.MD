# safehax/safefirmhax
## What this is:

Basically, it works like Brahma's arm9 bootstrap, but for a wider range of firmwares. It will try to load 'arm9.bin' (alternatively 'arm9loaderhax.bin') from the SD card root, and from there it'll load it into memory @ 0x23F00000. This is useful for a number of things, including installing otpless arm9loaderhax on N3DS, dumping your nand on the newer firmwares, and running CFW.

Currently, there's no support for 11.2 (at least until fasthax is usable).

## How it works:

Because 'SAFE_MODE' firm is out of date (~3.0 on O3DS, ~8.1 on N3DS), it's still vulnerable to firmlaunch-hax, which allows us to overwrite the arm9 entry pointer on firmlaunch. Knowing this, we can trigger a firmlaunch, so that 'SAFE_MODE' arm9 can run, then we sync up with arm9 until we can send another firmlaunch request to it. From there, we can do firmlaunch-hax like normal and gain arm9 code execution.

# Credits

 - Normmatt - Finding the vuln, helping work out an issue during KSync.
 - 'Everyone' - For also finding the [vuln](https://3dbrew.org/wiki/3DS_System_Flaws#Process9).
 - shinyquagsire23/patois(/etc?) - The [firmlaunch-hax](https://github.com/patois/Brahma) code that this uses snippets from, and was used as reference.
 - aliaspider/Mrrraou/etc. - [svchax](https://github.com/aliaspider/svchax)/[waithax](https://github.com/Mrrraou/waithax) (Credit to those who found these vulns as well).
 - [3DBrew](https://www.3dbrew.org/wiki/Main_Page)'s Users - VAddrs, and other useful information in general.
