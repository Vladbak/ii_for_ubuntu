if (!Function.prototype.bind) {
  Function.prototype.bind = function(oThis) {
    if (typeof this !== 'function') {
      // closest thing possible to the ECMAScript 5
      // internal IsCallable function
      throw new TypeError('Function.prototype.bind - what is trying to be bound is not callable');
    }

    var aArgs   = Array.prototype.slice.call(arguments, 1),
        fToBind = this,
        fNOP    = function() {},
        fBound  = function() {
          return fToBind.apply(this instanceof fNOP
                 ? this
                 : oThis,
                 aArgs.concat(Array.prototype.slice.call(arguments)));
        };

    if (this.prototype) {
      // Function.prototype doesn't have a prototype property
      fNOP.prototype = this.prototype; 
    }
    fBound.prototype = new fNOP();

    return fBound;
  };
}

(function() {
    var ownershipTokenPath = Script.resolvePath("ownershipToken.js");
    var tutorialPath = Script.resolvePath("tutorial.js");
    Script.include(ownershipTokenPath);
    Script.include(tutorialPath);

    var TutorialZone = function() {
        print("TutorialZone | Creating");
        this.token = null;
    };

    TutorialZone.prototype = {
        keyReleaseHandler: function(event) {
            print(event.text);
            if (event.isShifted && event.isAlt) {
                if (event.text == ",") {
                    if (!this.tutorialManager.startNextStep()) {
                        this.tutorialManager.startTutorial();
                    }
                } else if (event.text == "F11") {
                    this.tutorialManager.restartStep();
                } else if (event.text == "F10") {
                    MyAvatar.shouldRenderLocally = !MyAvatar.shouldRenderLocally;
                } else if (event.text == "r") {
                    this.tutorialManager.stopTutorial();
                    this.tutorialManager.startTutorial();
                }
            }
        },
        preload: function(entityID) {
            print("TutorialZone | Preload");
            this.entityID = entityID;
        },
        start: function() {
            print("TutorialZone | Got start");
            var self = this;
            if (!this.token) {
                print("TutorialZone | Creating token");
                this.token = new OwnershipToken(Math.random() * 100000, this.entityID, {
                    onGainedOwnership: function(token) {
                        print("TutorialZone | GOT OWNERSHIP");
                        if (!self.tutorialManager) {
                            self.tutorialManager = new TutorialManager();
                        }
                        self.tutorialManager.startTutorial();
                        print("TutorialZone | making bound release handler");
                        self.keyReleaseHandlerBound = self.keyReleaseHandler.bind(self);
                        print("TutorialZone | binding");
                        Controller.keyReleaseEvent.connect(self.keyReleaseHandlerBound);
                        print("TutorialZone | done");
                    },
                    onLostOwnership: function(token) {
                        print("TutorialZone | LOST OWNERSHIP");
                        if (self.tutorialManager) {
                            print("TutorialZone | stopping tutorial..");
                            self.tutorialManager.stopTutorial();
                            print("TutorialZone | done");
                            Controller.keyReleaseEvent.disconnect(self.keyReleaseHandlerBound);
                        } else {
                            print("TutorialZone | no tutorial manager...");
                        }
                    }
                });
            }
        },

        enterEntity: function() {
            print("TutorialZone | ENTERED THE TUTORIAL AREA");
        },
        leaveEntity: function() {
            print("TutorialZone | EXITED THE TUTORIAL AREA");
            if (this.token) {
                print("TutorialZone | Destroying token");
                this.token.destroy();
                this.token = null;
            }
        }
    };

    return new TutorialZone();
});
