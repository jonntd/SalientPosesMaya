#include <sstream>

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MStatus.h>

#include "OpenCLInfoCommand.hpp"
#include "SelectCommand.hpp"
#include "ReduceCommand.hpp"
#include "MayaUtils.hpp"

MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Richard Roberts", "0.2.0", "201810");
    
    status = plugin.registerCommand(OpenCLInfoCommand::kName, OpenCLInfoCommand::creator, OpenCLInfoCommand::newSyntax);
    if (status != MS::kSuccess) { Log::error(std::string(OpenCLInfoCommand::kName) + " failed to register"); }

	status = plugin.registerCommand(SelectCommand::kName, SelectCommand::creator, SelectCommand::newSyntax);
    if (status != MS::kSuccess) { Log::error(std::string(SelectCommand::kName) + " failed to register"); }

    status = plugin.registerCommand(ReduceCommand::kName, ReduceCommand::creator, ReduceCommand::newSyntax);
    if (status != MS::kSuccess) { Log::error(std::string(ReduceCommand::kName) + " failed to register"); }
    
	SelectCommand::openCLDirectory = plugin.loadPath();
    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);
    
    status = plugin.deregisterCommand(OpenCLInfoCommand::kName);
    if (status != MS::kSuccess) { Log::error(std::string(OpenCLInfoCommand::kName) + " failed to deregister"); }
    
	status = plugin.deregisterCommand(SelectCommand::kName);
    if (status != MS::kSuccess) { Log::error(std::string(SelectCommand::kName) + " failed to deregister"); }

    status = plugin.deregisterCommand(ReduceCommand::kName);
    if (status != MS::kSuccess) { Log::error(std::string(ReduceCommand::kName) + " failed to deregister"); }
    
    return status;
}
