CryEngine2 Source SDK Copyright(c) CryTek Corp. For NON COMMERCIAL Purposes only.

You may, free of charge, download and use the SDK to develop a modified CryTek game running on the CryEngine 2.
You may distribute your modified CryTek game in CryEngine 2 and object code form, but only for free.

````DISCLAIMER OF WARRANTIES. THE CryEngine 2 SDK AND ANY OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED "AS IS". CryTek AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, TITLE AND FITNESS FOR A PARTICULAR PURPOSE. LIMITATION OF LIABILITY. IN NO EVENT SHALL CryTek OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THE ENGINE AND/OR THE SDK, EVEN IF CRYTEK HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.````

If you would like to use the SDK for a commercial purpose, please contact CryTek at https://www.crytek.com/contact


## Info
Now that's out of the way, this is my attempt to port CryEngine 2 from VC8(C++03) over to VC14(C++20). I'm doing this for various reason: 
1. I work on MW:LL, and I work on it a lot, and a lot of the times, certain engine features are simply just broken and we can't safely/cleanly do what we want to do without having to rely on external libs, janky workarounds, and outdated coding standards.
2. Performance optimization: several underlying aspects of the engine severly limit the amount of players we can support per server. Being able to profile, check, and optimize underlying server code lets us stabilize server performance at high player count.
3. Educational purposes. I think its a great opportunity for me to learn about several key aspects of low-level and game development simultaneously, two things I'm very, very keenly interested in.
4. I'm simply sick of working in C++03. I want modern C++ features like `auto`, non-intrusive `smart_ptrs`, `unordered_map` to name off the top of my head. 
