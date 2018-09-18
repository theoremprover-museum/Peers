The Peers Theorem Prover

This is the code of the historic Peers theorem prover developed by Maria Paola Bonacina and
William W. (Bill) McCune at the Argonne National Laboratory in January-February 1993,
when Maria Paola was visiting Argonne after completing her PhD at the State University
of New York at Stony Brook in December 1992.

Maria Paola continued working on Peers during her postdoc at INRIA Lorraine in Nancy in
March-June 1993 and then from August 1993 through May 1995 at the University of Iowa,
that she joined as Assistant Professor in August 1993.

Peers was a parallel theorem prover based on the Clause-Diffusion methodology
that Maria Paola Bonacina invented and described in her PhD thesis:

Maria Paola Bonacina. Distributed automated deduction. Ph.D. Thesis, Department of Computer
Science, State University of New York at Stony Brook, December 1992.
Available at http://profs.sci.univr.it/~bonacina/pub_theses.html

Peers was a Clause-Diffusion prover for equational theories, possibly modulo associativity
and commutativity incorporating as sequential base code from Bill McCune's Otter Parts
Store (version of December 1992). Its name emphasizes that all processes in Clause-Diffusion
are peers, with no master-slave relation.

Peers was written in C and p4 for workstation networks.
This source code repository includes all .c files and .h files.
It also includes a makefile showing how the binary, named peers, was built.

Peers and its experiments up to March 1994 were presented in:

Maria Paola Bonacina and William W. McCune. Distributed theorem proving by Peers.
Proceedings of CADE-12, Springer, Lecture Notes in Artificial Intelligence 814,
841-845, 1994; DOI: 10.1007/3-540-58156-1_72. 

For further information on Clause-Diffusion see:
http://profs.sci.univr.it/~bonacina/distributed.html
For further information on the Clause-Diffusion provers see:
http://profs.sci.univr.it/~bonacina/cdprovers.html

