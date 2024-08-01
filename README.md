# HinaFlow
![HinaFlow logo](logo.png)

HinaFlow is a Houdini HDK based, open-source framework targeted at fluid simulation research in Computer Graphics and Machine Learning.

HinaFlow is a recursive acronym for "**H**inaFlow **i**s **n**ot **a** **F**luid **L**earning **O**ptimization **W**orkflow".

## Build Instructions

**Before building the project, make sure you have installed [Houdini](https://www.sidefx.com/).**

- First, clone the repository
    ```shell
    git clone https://github.com/HinaPE/HinaFlow.git
    cd HinaFlow
    ```
- Then, change `Houdini_PATH` in `FindHoudini.cmake` to your Houdini installation path.
- Build the project using CMake
    ```shell
    cmake -B build -S .
    cmake --build build
    ```
- Finally, open Houdini, create a DOP network, then you can find `HinaFlow` nodes in the tab menu under Digital Assets directory.


## References

### Papers
- Stam, Jos. “Stable fluids.” Proceedings of the 26th annual conference on Computer graphics and interactive techniques (1999): n. pag.
- Stam, Jos. “Real-Time Fluid Dynamics for Games.” (2003).
- Macklin, Miles and Matthias Müller. “Position based fluids.” ACM Transactions on Graphics (TOG) 32 (2013): 1 - 12.
- Weiler, Marcel, Dan Koschier and Jan Bender. “Projective fluids.” Proceedings of the 9th International Conference on Motion in Games (2016): n. pag.
- Gregson, James, Michael Krimerman, Matthias B. Hullin and Wolfgang Heidrich. “Stochastic tomography and its applications in 3D imaging of mixing fluids.” ACM Transactions on Graphics (TOG) 31 (2012): 1 - 10.


### Repositories
- [mantaflow](https://github.com/tum-pbs/mantaflow.git)
- [CubbyFlow](https://github.com/CubbyFlow/CubbyFlow.git)
- [SPlisHSPlasH](https://github.com/InteractiveComputerGraphics/SPlisHSPlasH.git)
