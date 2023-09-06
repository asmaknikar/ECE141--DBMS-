//
//  RemoteDatabase.hpp
//  RG_PA9
//
//  Created by rick gessner on 6/3/23.
//

#ifndef Remoting_h
#define Remoting_h

#include <sstream>
#include <string>
#include <iostream>
#include "unordered_map"
#include "BasicTypes.hpp"

namespace ECE141 {

  using SocketType = std::stringstream;

  class SocketProvider {
  public:
    virtual SocketType& getSocket()=0;
  };
  
  struct Channel {
    Channel(const std::string &aName) : name(aName) {}
    std::string name;
  };

  //---------------------------------
  class DBServer : public AsSingleton<DBServer> {
  public:

    bool registerClient(SocketProvider *aClient, const Channel &aChannel) {
      if(aChannel.name=="localhost") {
        clients[aClient]= std::make_unique<AppController>();
        return true;
      }
      return false;
    }

    void close(SocketProvider *aClient) {
      if(clients.count(aClient)) {
        clients.erase(aClient);
      }
    }

    StatusResult update(SocketProvider *aClient, ViewListener aListener) {
      StatusResult theResult{Errors::unknownCommand};
      if(clients.count(aClient)) {
        std::string theInput;
        std::getline(aClient->getSocket(), theInput);
        std::stringstream theCommand;
        theCommand << theInput;
        theResult=clients[aClient]->handleInput(theCommand, aListener);
      }
      return theResult;
    }

    std::map<SocketProvider*,std::unique_ptr<AppController>> clients;
    std::stringstream output;
  };

  //---------------------------------

  class DBConnector : public SocketProvider {
  public:

    DBConnector(const std::string &aChannelName) : state(ConnectState::disconnected), server(nullptr) {
      state=ConnectState::connecting;
      DBServer &theServer = DBServer::getInstance();
      Channel theChannel(aChannelName);
      if (theServer.registerClient(this, theChannel)) {
        server = &theServer;
        state = ConnectState::connected;
      }
      else state=ConnectState::disconnected;
    }

    ~DBConnector() {
      if (isConnected()) {
        state = ConnectState::disconnected;
        server->close(this);
      }
    }

    bool isConnected() { return state == ConnectState::connected; }

    StatusResult send(const std::string &aCmd, ViewListener aViewer) {
      StatusResult theResult;
      if (isConnected()) {
        socket << aCmd << "\n";
        theResult=server->update(this, aViewer);
        socket.str("");
      }
      return theResult;
    }
    
    SocketType& getSocket() override {
      return socket;
    }

  protected:

    enum class ConnectState {
      disconnected = 0, connecting, connected
    };

    ConnectState state;    
    SocketType  socket;
    DBServer    *server;
  };
}

#endif /* Remoting_h */

