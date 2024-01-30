# PSR (Partible State Replication) for VxWorks

Welcome to the PSR (Partible State Replication) project, a solo-developed prototype implemented for VxWorks.

## Overview

PSR is a exploration into state replication within distributed systems trageting control system. Developed as a concept test, this project includes a prototype implementation of VSR-QC (View Stamped Replication - Quorum Connected), aimed at maintaining consistency in distributed environments.

### Solo Development and Project Status

This project is the result of individual effort, and as such, it is full of quick-and-dirty solutions and shortcuts implemented to get the prototype up and running. While functional, the code reflects the --challenges and limitations of a one-person development process with very limited time.

### Disclaimer

Please note that this is an experimental implementation. If you choose to use or experiment with PSR, you do so at your own risk.

### OS Abstraction Layer

PSR is built with an OS abstraction layer, making it potentially easy to port to other systems. This aspect is particularly significant for those interested in adapting the project to different environments.

### Consensus protocol
PSR, as previously mentioned, relies on a distributed architecture. To achieve a coherent and consistent view across its network, it employs a consensus protocol. This protocol is grounded in Viewstamped Replication, albeit with modifications to enhance tolerance for partial connectivity. The key distinction between Viewstamped Replication (VSR) and VSR-QC lies in the leader election process. In VSR-QC, leaders are elected only if they are connected to a majority of peers, ensuring quorum connectivity.

A link to the PSR paper will be provided here for further reference. However, it is highly recommended for readers to first acquaint themselves with the seminal work by Liskov et al. on Viewstamped Replication. This foundational paper, titled "Viewstamped Replication Revisited," is accessible here: [Viewstamped replication revisted](https://dspace.mit.edu/bitstream/handle/1721.1/71763/MIT-CSAIL-TR-2012-021.pdf?sequence=1)



### Upcoming Paper

A detailed paper describing the PSR architecture is in the works and will be published soon. Once available, a link to the paper will be provided here for those interested in a deeper dive into the project's theoretical foundations.

## Getting Involved

Contributions to PSR are highly welcome, especially in the areas of code cleanup and refinement. The first step for contributors would be addressing the quick and dirty fixes to enhance stability and functionality.

To get started with PSR:

