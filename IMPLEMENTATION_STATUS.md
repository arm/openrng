# Implementation Status

This file lists the current implementation status of each generator and
distribution defined in the API.

### Generators

The list of available generators in the API is:

| Generator        | Implementation status   |
| :--------------- | :---------------------- |
| MCG31m1          | Implemented             |
| MCG59            | Implemented             |
| MRG32K3A         | Implemented             |
| MT19937          | Implemented             |
| SFMT19937        | Implemented             |
| MT2203           | Implemented             |
| R250             | Implemented             |
| PHILOX4x32x10    | Implemented             |
| ARS5             | Implemented             |
| SOBOL            | Implemented             |
| Nondeterministic | Implemented             |
| WH               | Not implemented         |
| NIEDERR          | Not implemented         |
| ABSTRACT         | Not implemented         |

All generators can produce random number in both single and double precision.

### Distributions

The available distributions in the API, both continuous and discrete, are:

| Distribution      | Implementation status   |
| :-----------------| :---------------------- |
| Uniform           | Implemented             |
| Gaussian          | Implemented             |
| GaussianMV        | Implemented             |
| Exponential       | Implemented             |
| Laplace           | Implemented             |
| Weibull           | Implemented             |
| Cauchy            | Implemented             |
| Rayleigh          | Implemented             |
| Lognormal         | Implemented             |
| Gumbel            | Implemented             |
| Uniform (int)     | Implemented             |
| UniformBits32     | Implemented             |
| UniformBits64     | Implemented             |
| Bernoulli         | Implemented             |
| Geometric         | Implemented             |
| Binomial          | Implemented             |
| Poisson           | Implemented             |
| Gamma             | Implemented             |
| Multinomial       | Implemented             |
| Beta              | Not Implemented         |
| Chi-Square        | Not Implemented         |
| Hypergeometric    | Not Implemented         |
| Negative binomial | Not Implemented         |
| PoissonV          | Not Implemented         |

All continuous distributions can produce values in single or double precisions
while the discrete distributions can only return integer values.