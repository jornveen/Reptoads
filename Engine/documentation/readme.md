# TBSG Documentation

This file shall give you help in order to navigate through our code base. The code base is split into 4 components:

1. Network lib
2. Server Application
3. Client Application
4. TBSGFramework



## Philosophy of the architecture

Since networking is such a big part of our framework we chose to make it the core way how we are thinking about a possible architecture. Network in general sends packages, operates on them and answers. Therefore we will try to create an architecture which supports this as much as possible.

This will be supported by the fact that we are turn based. This allows us to make use of an abstracted version of model view controller 

Source: [TDD](https://docs.google.com/document/d/1rLb9fJmYaXK4LVMNkPjwE1GHZj9qIGFKdyUiWga74ok/edit#bookmark=id.npbzlypxadsf)



## Design goals

1. Low level of indirections due better network package compatibility
2. Separation of logic (game logic) into the Server side and visualization in the client side
3. Event based approach due the command driven network architecture.
4. Models are operating on the provided network data and function as their brain / logic.



## Modules:

### NetworkLib

Contains the heart of the network architecture and is based on a ported version of [ENet](http://enet.bespin.org/)

### Server Application - Logic Part of the Game

This is the logic of our game the Server Application has the goal to operate uppon the data:

- React to the User input via the network
- Actually plays the game
  - Multiplayer
  - AI
- Is all knowing
- Keeps track of the Application data of each client
- Provides data for the client



### Client Application - Visual Part of the game

The actual game but the visual part. Its main aim is it to provide the interface for the user to play the game



### Framework

The binding parts of the architecture which are used by all the systems:



- Rendering
- Scripting
- Resource Loading / Parsing





