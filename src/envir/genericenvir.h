//==========================================================================
//  GENERICENVIR.H - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//
//  Declaration of the following classes:
//    EnvirBase:  abstract base class for simulation applications
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __OMNETPP_ENVIR_GENERICENVIR_H
#define __OMNETPP_ENVIR_GENERICENVIR_H

#include "omnetpp/carray.h"
#include "omnetpp/csimulation.h"
#include "omnetpp/cchannel.h"
#include "omnetpp/ccomponent.h"
#include "omnetpp/globals.h"
#include "omnetpp/cenvir.h"
#include "omnetpp/cexception.h"
#include "omnetpp/envirext.h"
#include "omnetpp/cconfiguration.h"
#include "omnetpp/cresultlistener.h"
#include "omnetpp/cevent.h"
#include "logformatter.h"
#include "args.h"
#include "envirdefs.h"

namespace omnetpp {

class cScheduler;
class cModuleType;
class cIListener;
class cProperty;
class cResultRecorder;
class cIEventlogManager;

namespace envir {

extern cConfigOption *CFGID_DEBUGGER_ATTACH_ON_ERROR;

class XMLDocCache;
class IFakeGUI;

/**
 * Default implementation of cEnvir. It was designed to only contain code necessary
 * for implementing the cEnvir interface.
 *
 * IN PARTICULAR, IT SHOULD *NEVER* BE EXTENDED WITH CODE RELATED TO SETTING UP AND
 * RUNNING SIMULATIONS. (Such code may go into AppBase).
 */
class ENVIR_API GenericEnvir : public cEnvir, protected cISimulationLifecycleListener
{
  protected:
    using cEnvir::simulation;

    // context
    ArgList *argList = nullptr;
    cConfiguration *cfg = nullptr;

    // log related
    LogFormatter logFormatter;
    bool logFormatUsesEventName = false;
    std::string currentEventName;
    bool logFormatUsesEventClassName = false;
    const char *currentEventClassName = nullptr;
    int currentModuleId = -1;

    // misc
    bool expressMode = true;  //TODO this is only used inside CmdEnvir
    bool gui = false;
    size_t extraStack = 0;

    // paths
    std::string nedPath;
    std::string nedExcludedPackages;
    std::string imagePath;

    bool printUndisposed = true;

    // Debugging. When set, cRuntimeError constructor executes a debug trap/launches debugger
    bool debugOnErrors = false;

    // Indicates whether eventlog recording is currently enabled (note: eventlogManager contains further filters)
    // It MUST be in sync with EventlogFileManager::isRecordingEnabled.
    bool recordEventlog = false;

    bool attachDebuggerOnErrors = false;

    // Output file managers, etc
    XMLDocCache *xmlCache = nullptr;
    cIEventlogManager *eventlogManager = nullptr;
    cIOutputVectorManager *outVectorManager = nullptr;
    cIOutputScalarManager *outScalarManager = nullptr;
    cISnapshotManager *snapshotManager = nullptr;

  protected:
    [[noreturn]] void unsupported(const char *method) const;

    // Called internally from readParameter(), to interactively prompt the
    // user for a parameter value.
    virtual void askParameter(cPar *par, bool unassigned);

    // Utility function for getXMLDocument() and getParsedXMLString()
    virtual cXMLElement *resolveXMLPath(cXMLElement *documentnode, const char *path);

    virtual void lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details) override;

  public:
    GenericEnvir();
    virtual ~GenericEnvir();
    virtual void setArgs(ArgList *args) {this->argList = args;}

    virtual void setSimulation(cSimulation *simulation) override;
    virtual void configure(cConfiguration *cfg) override;

    // getters/setters
    virtual cSimulation *getSimulation() const override {return simulation;}
    virtual cConfiguration *getConfig() override;

    virtual cIEventlogManager *getEventlogManager() const {return eventlogManager;}
    virtual void setEventlogManager(cIEventlogManager *obj);
    virtual cIOutputVectorManager *getOutVectorManager() const {return outVectorManager;}
    virtual void setOutVectorManager(cIOutputVectorManager *obj);
    virtual cIOutputScalarManager *getOutScalarManager() const {return outScalarManager;}
    virtual void setOutScalarManager(cIOutputScalarManager *obj);
    virtual cISnapshotManager *getSnapshotManager() const {return snapshotManager;}
    virtual void setSnapshotManager(cISnapshotManager *obj);

    virtual XMLDocCache *getXMLDocCache() const {return xmlCache;}

    virtual IFakeGUI *getFakeGui() const {return nullptr;}

    virtual void setExpressMode(bool enabled) {expressMode = enabled;}
    virtual void setIsGUI(bool enabled) {gui = enabled;}
    virtual void setExtraStackForEnvir(size_t size) {extraStack = size;}

    virtual bool getDebugOnErrors() const {return debugOnErrors;}
    virtual void setDebugOnErrors(bool enable) {debugOnErrors = enable;}
    virtual bool getAttachDebuggerOnErrors() const {return attachDebuggerOnErrors;}
    virtual void setAttachDebuggerOnErrors(bool enabled) {attachDebuggerOnErrors = enabled;}

    virtual void setLogLevel(LogLevel logLevel);
    virtual void setLogFormat(const char *logFormat);
    virtual LogFormatter& getLogFormatter() {return logFormatter;}

    virtual void setEventlogRecording(bool enabled);  //TODO note: currently this does suspend()/resume(), which is mostly only useful for Qtenv
    virtual bool getEventlogRecording() const {return recordEventlog;}

    virtual bool getCheckSignals() const {return cComponent::getCheckSignals();}
    virtual void setCheckSignals(bool checkSignals) {cComponent::setCheckSignals(checkSignals);}
    virtual const char *getImagePath() const {return imagePath.c_str();}
    virtual void setImagePath(const char *imagePath) {this->imagePath = imagePath;}
    virtual bool getPrintUndisposed() const {return printUndisposed;}
    virtual void setPrintUndisposed(bool printUndisposed) {this->printUndisposed = printUndisposed;}

    virtual std::string extractImagePath(cConfiguration *cfg, ArgList *args);

    virtual void clearCurrentEventInfo();

    // eventlog callback interface
    virtual void objectDeleted(cObject *object) override;
    virtual void simulationEvent(cEvent *event) override;
    virtual void componentInitBegin(cComponent *component, int stage) override;
    virtual void messageScheduled(cMessage *msg) override;
    virtual void messageCancelled(cMessage *msg) override;
    virtual void beginSend(cMessage *msg, const SendOptions& options) override;
    virtual void messageSendDirect(cMessage *msg, cGate *toGate, const ChannelResult& result) override;
    virtual void messageSendHop(cMessage *msg, cGate *srcGate) override;
    virtual void messageSendHop(cMessage *msg, cGate *srcGate, const cChannel::Result& result) override;
    virtual void endSend(cMessage *msg) override;
    virtual void messageCreated(cMessage *msg) override;
    virtual void messageCloned(cMessage *msg, cMessage *clone) override;
    virtual void messageDeleted(cMessage *msg) override;
    virtual void moduleReparented(cModule *module, cModule *oldparent, int oldId) override;
    virtual void componentMethodBegin(cComponent *from, cComponent *to, const char *methodFmt, va_list va, bool silent) override;
    virtual void componentMethodEnd() override;
    virtual void moduleCreated(cModule *newmodule) override;
    virtual void moduleDeleted(cModule *module) override;
    virtual void gateCreated(cGate *newgate) override;
    virtual void gateDeleted(cGate *gate) override;
    virtual void connectionCreated(cGate *srcgate) override;
    virtual void connectionDeleted(cGate *srcgate) override;
    virtual void displayStringChanged(cComponent *component) override;
    virtual void undisposedObject(cObject *obj) override;
    virtual void log(cLogEntry *entry) override;

    virtual const char *getCurrentEventName() override { return logFormatUsesEventName ? currentEventName.c_str() : nullptr; }
    virtual const char *getCurrentEventClassName() override { return currentEventClassName; }
    virtual cModule *getCurrentEventModule() override { return currentModuleId != -1 ? getSimulation()->getModule(currentModuleId) : nullptr; }

    // configuration, model parameters
    virtual void preconfigureComponent(cComponent *component) override;
    virtual void configureComponent(cComponent *component) override;
    virtual void readParameter(cPar *parameter) override;
    virtual cXMLElement *getXMLDocument(const char *filename, const char *xpath=nullptr) override;
    virtual cXMLElement *getParsedXMLString(const char *content, const char *xpath=nullptr) override;
    virtual void forgetXMLDocument(const char *filename) override;
    virtual void forgetParsedXMLString(const char *content) override;
    virtual void flushXMLDocumentCache() override;
    virtual void flushXMLParsedContentCache() override;
    virtual unsigned getExtraStackForEnvir() const override;
    virtual std::string resolveResourcePath(const char *fileName, cComponentType *context) override;

    // UI functions
    virtual void bubble(cComponent *component, const char *text) override;

    // output vectors
    virtual void *registerOutputVector(const char *modulename, const char *vectorname) override;
    virtual void deregisterOutputVector(void *vechandle) override;
    virtual void setVectorAttribute(void *vechandle, const char *name, const char *value) override;
    virtual bool recordInOutputVector(void *vechandle, simtime_t t, double value) override;

    // output scalars
    virtual void recordScalar(cComponent *component, const char *name, double value, opp_string_map *attributes=nullptr) override;
    virtual void recordStatistic(cComponent *component, const char *name, cStatistic *statistic, opp_string_map *attributes=nullptr) override;
    virtual void recordParameter(cPar *par) override;
    virtual void recordComponentType(cComponent *component) override;

    virtual void addResultRecorders(cComponent *component, simsignal_t signal, const char *statisticName, cProperty *statisticTemplateProperty) override;

    // snapshot file
    virtual std::ostream *getStreamForSnapshot() override;
    virtual void releaseStreamForSnapshot(std::ostream *os) override;

    // misc
    virtual void refOsgNode(osg::Node *scene) override {}
    virtual void unrefOsgNode(osg::Node *scene) override {}
    virtual bool idle() override;

    virtual bool ensureDebugger(cRuntimeError *error = nullptr) override;
    virtual bool shouldDebugNow(cRuntimeError *error = nullptr) override;

    virtual bool isGUI() const override;
    virtual bool isExpressMode() const override;
    virtual void alert(const char *msg) override;
    virtual std::string input(const char *prompt, const char *defaultReply=nullptr) override;
    virtual bool askYesNo(const char *prompt) override;
    virtual void getImageSize(const char *imageName, double& outWidth, double& outHeight) override;
    virtual void getTextExtent(const cFigure::Font& font, const char *text, double& outWidth, double& outHeight, double& outAscent) override;
    virtual void appendToImagePath(const char *directory) override;
    virtual void loadImage(const char *fileName, const char *imageName=nullptr) override;
    virtual cFigure::Rectangle getSubmoduleBounds(const cModule *submodule) override;
    virtual std::vector<cFigure::Point> getConnectionLine(const cGate *sourceGate) override;
    virtual double getZoomLevel(const cModule *module) override;
    virtual double getAnimationTime() const override;
    virtual double getAnimationSpeed() const override;
    virtual double getRemainingAnimationHoldTime() const override;
    virtual void pausePoint() override;
};

}  // namespace envir
}  // namespace omnetpp

#endif

