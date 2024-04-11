# adaptivetau: Tau-Leaping Stochastic Simulation

adaptivetau is a [R](https://www.r-project.org) package that implements adaptive tau leaping to approximate the trajectory of a continuous-time stochastic process as described by [Cao et al. (2007) The Journal of Chemical Physics](https://doi.org/10.1063%2F1.2745299).  The core of the algorithm is implemented in C++, so it is *much* more efficient than a pure-R implementation.

See NEWS file for package history; code was transferred from subversion to git in Apr 2024.

#### Installation

Install as any R package from [CRAN](https://CRAN.R-project.org/package=adaptivetau) using
```r
install.packages("adaptivetau")
```

If you want to install from this git repository, from the directory above this, first build an R package:
```
R CMD build adaptivetau
```
The resulting package will be named adaptivetau_XXX.tar.gz where XXX is replaced with the current R package version number found in the DESCRIPTION.  This file can then be installed as with any local R package installed from source.

#### Usage

Usage information can be found in the vignette (which gets compiled into a PDF when the package is built) and R help pages.


#### Bug reports
Developers or users reporting bugs/feature requests shoulds create an issue on [github](https://github.com/plfjohnson/adaptivetau/issues).
