# HinaFlow

![HinaFlow logo](logo.png)

HinaFlow is a Houdini HDK based, open-source framework targeted at fluid simulation research in Computer Graphics and Machine Learning.

HinaFlow is a recursive acronym for "**H**inaFlow **i**s **n**ot **a** **F**luid **L**earning **O**ptimization **W**orkflow".

## Key Features

**Differentiable Fluid Solver and Even Deep Learning based Fluid Solver in Houdini**

HinaFlow runs on two contexts: Houdini and Python. Either context uses Houdini hdk nodes. (No Houdini's Python node)
- When running on pure Houdini, you can purely use HinaFlow's HDK nodes as a extended Houdini Digital Asset. There is no difference from the original Houdini workflow.
- When running on Python context, you can take full advantage of the whole Python ecosystem, including PyTorch, TensorFlow, NumPy, etc, and build your own fluid simulation pipeline.
- The two contexts are fully compatible with each other. You can switch between them at any time.

For example, you can easily create a **Differentiable Fluid Solver** using PyTorch, or even create a **FluidGAN** using PyTorch.

In Hinaflow, we mainly use [phiflow](https://github.com/tum-pbs/PhiFlow.git) as the backend for fluid simulation, which is a fully differentiable fluid simulation framework, with PyTorch, JAX and TensorFlow backends.

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

**Setup python(hython) context for Houdini**

If you want to use python context, you need to install the external dependencies for houdini built-in python (i.e. hython)

- Install pip for hython, and install torch and phiflow (or other necessary packages you want)
    ```shell
    curl https://bootstrap.pypa.io/get-pip.py -o ./get-pip.py
    hython get-pip.py
    hython -m pip install --upgrade pip setuptools
    hython -m pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121 # Note: change torch version according to your cuda version
    hython -m pip install phiflow
    ```

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
- [phiflow](https://github.com/tum-pbs/PhiFlow.git)
- [mantaflow](https://github.com/tum-pbs/mantaflow.git)
- [CubbyFlow](https://github.com/CubbyFlow/CubbyFlow.git)
- [SPlisHSPlasH](https://github.com/InteractiveComputerGraphics/SPlisHSPlasH.git)
