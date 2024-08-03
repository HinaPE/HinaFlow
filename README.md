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

#### Course Notes

- Bridson, Robert and Matthias Müller-Fischer. “Fluid simulation: SIGGRAPH 2007 course notesVideo files associated with this course are available from the citation page.” ACM SIGGRAPH 2007 courses (2007): n. pag.
- Ihmsen, Markus, Jens Orthmann, Barbara Solenthaler, Andreas Kolb and Matthias Teschner. “SPH Fluids in Computer Graphics.” Eurographics (2014).

#### Eulerian Fluids

- Stam, Jos. “Stable fluids.” Proceedings of the 26th annual conference on Computer graphics and interactive techniques (1999): n. pag.
- Stam, Jos. “Real-Time Fluid Dynamics for Games.” (2003).

#### Lagrangian Fluids

- Macklin, Miles and Matthias Müller. “Position based fluids.” ACM Transactions on Graphics (TOG) 32 (2013): 1 - 12.
- Weiler, Marcel, Dan Koschier and Jan Bender. “Projective fluids.” Proceedings of the 9th International Conference on Motion in Games (2016): n. pag.

#### Learning/Optimisation Fluids
- Gregson, James, Michael Krimerman, Matthias B. Hullin and Wolfgang Heidrich. “Stochastic tomography and its applications in 3D imaging of mixing fluids.” ACM Transactions on Graphics (TOG) 31 (2012): 1 - 10.
- Eckert, Marie-Lena, Wolfgang Heidrich and Nils Thürey. “Coupled Fluid Density and Motion from Single Views.” Computer Graphics Forum 37 (2018): n. pag.
- Inglis, Tiffany, Marie-Lena Eckert, James Gregson and Nils Thürey. “Primal‐Dual Optimization for Fluids.” Computer Graphics Forum 36 (2016): n. pag.
- Eckert, Marie-Lena. “Optimization for Fluid Simulation and Reconstruction of Real-World Flow Phenomena (Optimierung für Fluidsimulationen und Rekonstruktion von realen Strömungsphänomenen).” (2019).

### Repositories

- [mantaflow](https://github.com/tum-pbs/mantaflow.git)
- [CubbyFlow](https://github.com/CubbyFlow/CubbyFlow.git)
- [SPlisHSPlasH](https://github.com/InteractiveComputerGraphics/SPlisHSPlasH.git)
