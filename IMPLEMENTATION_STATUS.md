# Implementation Status

This file lists the current implementation status of each generator and
distribution defined in the API.

### Generators

The list of available generators in the API is:

| Generator        | Implementation status   |
| :--------------- | :---------------------- |
| ABSTRACT         | Not implemented         |
| ARS5             | Implemented             |
| MCG31m1          | Implemented             |
| MCG59            | Implemented             |
| MRG32K3A         | Implemented             |
| MT19937          | Implemented             |
| MT2203           | Implemented             |
| NIEDERR          | Implemented             |
| Nondeterministic | Implemented             |
| PHILOX4x32x10    | Implemented             |
| R250             | Implemented             |
| SFMT19937        | Implemented             |
| SOBOL            | Implemented             |
| WH               | Not implemented         |

All generators can produce random number in both single and double precision.

### Distributions

The available distributions in the API, both continuous and discrete, are:

| Distribution      | Implementation status   |
| :-----------------| :---------------------- |
| Bernoulli         | Implemented             |
| Beta              | Not Implemented         |
| Binomial          | Implemented             |
| Cauchy            | Implemented             |
| Chi-Square        | Implemented             |
| Exponential       | Implemented             |
| Gamma             | Implemented             |
| Gaussian          | Implemented             |
| GaussianMV        | Implemented             |
| Geometric         | Implemented             |
| Gumbel            | Implemented             |
| Hypergeometric    | Not Implemented         |
| Laplace           | Implemented             |
| Lognormal         | Implemented             |
| Multinomial       | Implemented             |
| Negative binomial | Not Implemented         |
| Poisson           | Implemented             |
| PoissonV          | Implemented             |
| Rayleigh          | Implemented             |
| Uniform           | Implemented             |
| Uniform (int)     | Implemented             |
| UniformBits32     | Implemented             |
| UniformBits64     | Implemented             |
| Weibull           | Implemented             |

All continuous distributions can produce values in single or double precisions
while the discrete distributions can only return integer values.