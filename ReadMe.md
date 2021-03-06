# What is it?

- An external for MAX/MSP. 

- A signal rate, "stochastic" rhythm generator. 

- An impressionistic take on the idea of "hyperuniformity in the disordered, jammed, packing of soft spheres" as outlined in [this paper](https://arxiv.org/pdf/1402.6058). (As to why "impressionistic" see the section "Hyperuniform" below.)

### DISCLAIMER
Sorry - the MAX patch is currently a bit cryptic and weird, and nothing is documented very well. I put this together very quickly and I will have no time over the next few weeks to make it easier to understand. It isn't a complicated algorithm, but the reasoning, controls and possibilities might be opaque. If you are lost, then accept my apologies, and please bear with me, I'll improve this when I have time. My current research is not in computer music and I have a couple of other projects I need to finish up more urgently.

**Warning: compiling currently seems sketchy on mac - at least one person has had problems getting this to work. I'm working through those issues at present.**

# What now?
Here's an image to help describe how it works:

Imagine a funnel or-hourglass type vessel - a wide reservoir at the top, narrowing to a thinner delivery tube. The "funnel" is filled with soft(ish) spheres - like marbles, only more-or-less squishy. There are n different-sized types of soft sphere in the funnel, each with variable softness and diameter. These fall down the funnel and onto a conveyor belt which is only wide enough for 1 sphere. As they fall onto the conveyor belt, the crowdedness of the population is such that the spheres jam together. The spheres therefore get deformed - squeezed, some more, some less, some barely at all, depending on the chance manner in which they fall. The rhythm is generated by taking the width of each squeezed sphere on the conveyor belt as the length of a note/ beat. 

# Ok. Why?
First off, the overall shape of the thing is appealing to me: a fixed set of discrete units of time is strung together, and deformed, and this happens in a controllably random way. It avoids the grid, and strong repetition, while retaining patterning that the ear can follow. 

This seems to me to be a potentially interesting way of organising musical time. I think it offers the potential for a fairly intuitive "loose" control of rhythm, by modulating probabilities in time (e.g. where you'd usually send note triggers, you could send probability curves). 

Finally, I think you could probably remove/diminish the random element and keep the "stringing together and deforming spheres" element, to build a tool for intuitive direct, performance control of rhythm. It wouldn't take much to adapt the code.

Anyway, after a few hours playing with the results, I think it's quite a controllable and flexible method of generating rhythms. 

On top of this, the process fits my intuitions about extended-rhythm playing as it evades the canonically rigid time grid.  In particular I think it the echoes the way some players and musical cultures sometimes seem to organise their more-or-less-free rhythmic percussion playing: by chaining together and flexibly deforming units of time. 

I think the model also fits well with up to date neuroscience on rhythm perception. Research suggests this is best modelled as both pulse-based and stochastic; it is thought that we anticipate the next rhythmic event based on previous events, but via a stochastic estimation mechanism (see e.g. [*Rhythm Music and the Brain* by Thaut](https://www.amazon.co.uk/Rhythm-Music-Brain-Foundations-Applications/dp/041596475X?tag=ad-backfill-amzn-no-or-one-good-20)).

# Off Grid Rhythm?
If you are interested in off-grid rhythm in computer music, this is not my first effort in this area. I have also tackled this using Central Pattern Generators - neural networks found in mammals and other animals, which control heartbeat, lung function and other adaptive rhythmic behaviour. [See my paper here](https://www.researchgate.net/publication/324360165_Neurythmic_A_Rhythm_Creation_Tool_Based_on_Central_Pattern_Generators). That software will also be made available soon)). 

# "Hyperuniform"?
I say above that this is an *impressionistic* take on the way  "hyperuniform" patterns emerge from the disordered, jammed, packing of soft spheres. It is impressionistic in the sense that the patterns here are likely not hyperuniform, and nor does the arrangement of spheres likely fit the strict materials science definition of "jammed". Rather, I started from the image, and the broad concept and worked towards something I found musically interesting. You can see the original idea described in [this paper](https://arxiv.org/pdf/1402.6058). I am sure that more rigorous takes on the process desribed in that paper are likely to be interesting. My instinct is that the full 2D version presented there is likely to need to be generated offline, but it might be fun to navigate a pre-generated distribution in realtime. There is probably also room for other takes on the idea, which, like mine, are 1D and controllable in realtime, but which are in some way "more rigorous" (e.g. by actually adapting the equations in the paper). There are probably interesting results to be had there, and it would be interesting to see how much they differ perceptually from my approach. If you have a go, please tell me about it. I'd be really interested. 




------------------------------------------------------------------------------------------------------------------------
# BUILD INSTRUCTIONS: COPIED FROM MIN DEVKIT

## Prerequisites

To build the externals in this package you will need some form of compiler support on your system. 

* On the Mac this means **Xcode 7 or later** (you can get from the App Store for free). 
* On Windows this means **Visual Studio 2015 or 2017** (you can download a free version from Microsoft). The installer for Visual Studio 2017 offers an option to install Git, which you should choose to do.

You will also need to install a recent version of [CMake](https://cmake.org/download/).


## Building

0. Clone this repo from Github, into Max's Packages folder.
   The *Packages* folder can be found inside of your *Max 7* or *Max 8* folder that is inside of your user's *Documents* folder.
1. In the Terminal or Console app of your choice, change directories (cd) into the `hyperuniform` folder you cloned/installed in step 0.
2. `mkdir build` to create a folder with your various build files
3. `cd build` to put yourself into that folder
4. Now you can generate the projects for your choosen build environment:

### Mac 

Run `cmake -G Xcode ..` and then run `cmake --build .` or open the Xcode project from this "build" folder and use the GUI.

Note: you can add the `-j4` option where "4" is the number of cores to use.  This can help to speed up your builds, though sometimes the error output is interleaved in such a way as to make troubleshooting more difficult.

### Windows

You can run `cmake --help` to get a list of the options available.  Assuming some version of Visual Studio 2017, the commands to generate the projects will look like this:

* 32 bit: `cmake -G "Visual Studio 15 2017" ..`
* 64 bit: `cmake -G "Visual Studio 15 2017 Win64" ..`

Having generated the projects, you can now build by opening the .sln file in the build folder with the Visual Studio app (just double-click the .sln file) or you can build on the command line like this:

`cmake --build . --config Release`


## Unit Testing

On the command like you can run all unit tests using Cmake:

* on debug builds: `ctest -C Debug .`
* on release builds: `ctest -C Release .`

Or you can run an individual test, which is simply a command line program:

* `cd ..`
* `cd tests`
* mac example: `./test_dcblocker_tilde -s`
* win example: `test_dcblocker_tilde.exe -s`

Or run them your IDE's debugger.



## Contributors / Acknowledgements

* See the [GitHub Contributor Graph](https://github.com/Cycling74/min-api/graphs/contributors) for the API
* See the [GitHub Contributor Graph](https://github.com/Cycling74/min-devkit/graphs/contributors) for the DevKit

## Support

For support, please use the developer forums at:
http://cycling74.com/forums/
