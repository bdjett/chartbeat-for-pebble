//////////////////////////////////////////////////////////
// Chartbeat for Pebble
// Monitor your dashboard on your wrist
//
// Created by: Brian Jett on 18/12/2014
// 

var apiKey = "317a25eccba186e0f6b558f45214c0e7";
var host = "gizmodo.com";

var maxVisits = 0;

var maxAppMessageTries = 3;
var appMessageRetryTimeout = 3000;
var appMessageTimeout = 0;
var httpTimeout = 10000;
var appMessageQueue = [];

var sendAppMessage = function() {
    if (appMessageQueue.length > 0) {
        var currentAppMessage = appMessageQueue[0];
        currentAppMessage.numTries = currentAppMessage.numTries || 0;
        currentAppMessage.transactionId = currentAppMessage.transactionId || -1;

        if (currentAppMessage.numTries < maxAppMessageTries) {
            Pebble.sendAppMessage(
                currentAppMessage.message,
                function(e) {
                    appMessageQueue.shift();
                    setTimeout(function() {
                        sendAppMessage();
                    }, appMessageTimeout);
                }, function(e) {
                    console.log("Faled sending AppMessage for transactionId: " + e.data.transactionId + ". Error: " + e.data.error.message);
                    appMessageQueue[0].transactionId = e.data.transactionId;
                    appMessageQueue[0].numTries++;
                    setTimeout(function() {
                        sendAppMessage();
                    }, appMessageRetryTimeout);
                }
            );
        } else {
            console.log("Faled sending AppMessage after multiple attemps for transactionId: " + currentAppMessage.transactionId + ". Error: None. Here's the message: " + JSON.stringify(currentAppMessage.message));
        }
    }
};

function numberWithCommas(x) {
    return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}

// getMaxVisits()
var getMaxVisits = function() {
  var url = "http://api.chartbeat.com/historical/traffic/stats/?apikey=" + apiKey + "&host=" + host;
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if (req.status == 200) {
        if (req.responseText) {
          var response = JSON.parse(req.responseText);
          var max = response.data[host].people.max;
          maxVisits = max;
        }
      }
    }
  };
  req.ontimeout = function() {
    
  };
  req.onerror = function() {
    
  };
  req.send();
};

// getCurrentDashboard()
// Poll the Chartbeat API to get the most recent statistics
var getCurrentDashboard = function() {
  var url = "http://api.chartbeat.com/live/quickstats/v4/?apikey=" + apiKey + "&host=" + host;
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if (req.status == 200) {
        if (req.responseText) {
          var response = JSON.parse(req.responseText);
          var people = response.data.stats.people;
          var message = {'people': people, 'peopleString': numberWithCommas(people).toString(), 'max': maxVisits};
          console.log(JSON.stringify(message));
          appMessageQueue.push({'message': message});
          sendAppMessage();
        }
      }
    }
  };
  req.ontimeout = function() {
    
  };
  req.onerror = function() {
    
  };
  req.send();
};

Pebble.addEventListener('ready', function(e) {
  //getCurrentDashboard();
  getMaxVisits();
});

// RECEIVED APP MESSAGE
Pebble.addEventListener("appmessage", function(e) {
  console.log("Received message: " + JSON.stringify(e));
  if (e.payload.getData) {
    // GET WAIT TIMES
    getCurrentDashboard();
  }
});