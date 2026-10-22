Jeevan Marg

Smart Emergency Response and Assistance System

A modular emergency-response project designed to connect emergency
dispatch, healthcare discovery, navigation, and a unified dashboard
for Dehradun.

Overview

During an emergency, people may need to report an incident, identify a
suitable healthcare facility, and find a route to assistance. ResQ
Dehradun aims to bring these activities into one coordinated system.

The project is organized into four modules that can be developed and
tested independently, then connected through defined interfaces.

Problem Statement

Emergency-related information can be scattered across different sources.
Finding the right contact, understanding available medical facilities,
and identifying a route may take valuable time. ResQ Dehradun explores
how a connected application can organize these tasks in a single
workflow.

Objectives

Organize and prioritize emergency requests.

Help users discover hospitals and medical facilities.

Support route and destination discovery.

Connect the modules through documented interfaces and APIs.

Build a modular, maintainable, and testable application.

Clearly distinguish implemented functionality from planned features.

System Modules

M1 --- Emergency Dispatch

Purpose: Organize incoming emergency requests and support response
prioritization.

Responsibilities: - Capture and structure emergency request details. -
Track request information and status. - Apply defined urgency or
priority rules. - Coordinate relevant information with healthcare and
navigation modules. - Make request information available to the
integrated dashboard.

Intended outcome: A structured emergency request that can move
through the ResQ workflow.

M2 --- Healthcare Discovery and Assistance

Purpose: Organize healthcare-facility information for discovery
during an emergency.

Responsibilities: - Represent hospitals, doctors, departments, medical
services, and bed availability. - Search and filter hospitals using
supported criteria. - Sort results by attributes such as rating or
available beds. - Store and retrieve hospital data through a repository
layer. - Provide healthcare information to connected modules through a
defined interface or API.

Intended outcome: Structured hospital information and search results
that can support emergency decision-making.

Sample hospital data must be labelled as illustrative. Real-world use
requires verified and up-to-date data.

M3 --- Navigation and Routing

Purpose: Help identify destinations and routes to emergency
resources.

Responsibilities: - Represent relevant locations and connections. -
Identify nearby emergency resources when suitable location data is
available. - Find or retrieve routes to selected destinations. - Apply
graph and pathfinding techniques where appropriate. - Share destination
and route information with other modules.

Intended outcome: Route and destination information to support
reaching an emergency resource.

Route quality depends on the accuracy and freshness of the underlying
map and location data.

M4 --- Frontend Dashboard and Integration

Purpose: Provide a common interface and connect the functionality
developed by the other modules.

Responsibilities: - Present emergency and healthcare information
clearly. - Connect the frontend to backend services or APIs. - Display
hospital search results and routing information when integrations are
available. - Coordinate information flow across modules. - Handle
loading, empty-result, and error states.

Intended outcome: A unified interface for accessing the capabilities
of the integrated ResQ system.

High-Level Workflow

A user reports or initiates an emergency.

M1 --- Emergency Dispatch records the request and applies the
defined prioritization logic.

M2 --- Healthcare Discovery supplies relevant hospital and
medical-facility information.

M3 --- Navigation and Routing helps identify a destination and
route using available data.

M4 --- Frontend Dashboard and Integration presents available
information through a common interface.

The modules exchange information through their defined interfaces as
integration progresses.

The actual workflow depends on the features implemented and verified in
each module.

Proposed Architecture

                   ResQ Dehradun
                         |
             Frontend / Dashboard (M4)
                         |
                API / Integration Layer
                         |
       +-----------------+-----------------+
       |                 |                 |
       v                 v                 v
 M1: Emergency      M2: Healthcare    M3: Navigation
    Dispatch          Discovery         and Routing
       |                 |                 |
 Request handling   Hospital data     Locations, routes,
 and prioritization and search        and pathfinding

This is the intended logical architecture; it does not mean every
connection or API is already implemented.

Technology Stack

The exact stack may evolve as the modules are integrated.

Area                                Technology / Approach

Core implementation                 C++

C++ language standard               C++17 where configured

Data structures and algorithms      Hash maps, priority queues, graphs,
searching, sorting, and pathfinding
as appropriate

Healthcare persistence              SQLite planned for persistent
hospital storage

Inter-module communication          REST-style APIs planned

Frontend                            Dashboard/interface module;
document the chosen framework as
confirmed

Build                               Makefile planned for reproducible
C++ builds

Version control                     Git and GitHub

Only describe a technology as implemented after it has been configured
and verified in the repository.


