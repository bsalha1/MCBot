# MCBot
This is a bot that communicates with Minecraft servers using packets in C++ using the Windows 10 TCP stack.

# Outline
This program operates on two levels: the *Packet Layer* and the *Bot Layer* which are explained below.

### Packet Layer
Every packet in the Minecraft protocol can be sent with any parameters one specifies. This means you can send ridiculous commands. This layer is not controlled by anything is meant to be implemented any way one desires.

### Bot Layer
This layer implements the *Packet Layer* to send packets in a meaningful order and in contexts to mimic an actual client. It does this through the 3 following methods:
- **ReceiverThread**: receives packets every 1 millisecond and dispatches the necessary actions
- **PositionThread**: sends position, yaw and pitch of player every tick (20 times per second)
- **Interaction Methods**: sends meaningful packets that mimic the client. For example one can dispatch a movement routine, attack a monster, open a chest etc.

# NuGet Packages
**Boost**  
**OpenSSL**  
**Nholmann.Json**  
**Zlib**  

