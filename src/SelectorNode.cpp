//
//  SelectorNode.cpp
//  SalientPosesMaya
//
//  Created by Richard Roberts on 3/04/18.
//

#include <sstream>
#include <vector>

#include <maya/MStatus.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MIntArray.h>
#include <maya/MFnFloatArrayData.h>
#include <maya/MFloatArray.h>

#include "MayaUtils.hpp"
#include "../SalientPosesPerformance/src/ErrorTable.hpp"
#include "../SalientPosesPerformance/src/Selector.hpp"
#include "SelectorNode.hpp"

MTypeId SelectorNode::id( 0x0012c2c1 );

MObject SelectorNode::iaStart;
MObject SelectorNode::iaEnd;
MObject SelectorNode::iaStartOffset;
MObject SelectorNode::iaNFramesTotal;
MObject SelectorNode::iaErrorTable;
MObject SelectorNode::iaNKeyframes;
MObject SelectorNode::oaSelectionErrors;
MObject SelectorNode::oaSelection;

MStatus SelectorNode::initialize() {
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    
    oaSelectionErrors = tAttr.create("errors", "es", MFnData::kFloatArray);
    tAttr.setWritable(false);
    tAttr.setReadable(true);
    tAttr.setStorable(true);
    addAttribute(oaSelectionErrors);
    
    oaSelection = tAttr.create("selection", "sel", MFnData::kIntArray);
    tAttr.setWritable(false);
    tAttr.setReadable(true);
    tAttr.setStorable(true);
    addAttribute(oaSelection);
    
    iaStart = nAttr.create("start", "s", MFnNumericData::kInt);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setStorable(true);
    addAttribute(iaStart);
    attributeAffects(iaStart, oaSelection);
    
    iaEnd = nAttr.create("end", "e", MFnNumericData::kInt);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setStorable(true);
    addAttribute(iaEnd);
    attributeAffects(iaEnd, oaSelection);
    
    iaStartOffset = nAttr.create("startOffset", "o", MFnNumericData::kInt);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setStorable(true);
    addAttribute(iaStartOffset);
    attributeAffects(iaStartOffset, oaSelection);
    
    iaNFramesTotal = nAttr.create("framesTotal", "ft", MFnNumericData::kInt);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setStorable(true);
    addAttribute(iaNFramesTotal);
    attributeAffects(iaNFramesTotal, oaSelection);
    
    iaErrorTable = tAttr.create("errorTable", "et", MFnData::kFloatArray);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setStorable(true);
    addAttribute(iaErrorTable);
    attributeAffects(iaErrorTable, oaSelection);
    
    iaNKeyframes = nAttr.create("nKeyframes", "n", MFnNumericData::kInt);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setStorable(true);
    addAttribute(iaNKeyframes);
    attributeAffects(iaNKeyframes, oaSelection);
    
    return MS::kSuccess;
}

MStatus SelectorNode::compute(const MPlug& plug, MDataBlock& data) {
    MStatus status;
    
    if (plug == oaSelection || plug == oaSelectionErrors) {
        Log::print("COMPUTING SELECTIONS");
        
        int startOffset = data.inputValue(iaStartOffset).asInt();
        int nFramesTotal = data.inputValue(iaNFramesTotal).asInt();
        int selStart = data.inputValue(iaStart).asInt();
        int selEnd = data.inputValue(iaEnd).asInt();
        
        int animStart = selStart - startOffset;
        int animEnd = animStart + nFramesTotal - 1;
        int nFrames = selEnd - selStart + 1;
        int nFramesPlus1 = nFrames + 1;
        int nFramesPlus1Sq = nFrames * nFramesPlus1;
        
        // Extract previously computed error data from handles
        MDataHandle errorTableHandle = data.inputValue(iaErrorTable);
        MObject errorTableObject = errorTableHandle.data();
        MayaCheck::objectIsFloatArray(errorTableObject);
        MFnFloatArrayData errorTableData(errorTableObject, &status);
        if (status != MS::kSuccess) {
            Log::showStatusWhenError(status, "Failed to error table");
            return MS::kFailure;
        }
        
        MFloatArray errorTableArray = errorTableData.array();
        std::vector<float> errorData(errorTableArray.length(), -1);
        for (int i = 0; i < errorTableArray.length(); i++) {
            errorData[i] = errorTableArray[i];
        }
        
        // Genereate the selections
        
        
        AnimationProxy anim = AnimationProxy::justStartAndEnd(animStart, animEnd);
        ErrorTable analysis = ErrorTable::fromData(errorData, nFramesTotal, anim.start, anim.end);
        Selector selector = Selector::fromAnal(anim, analysis, selStart, selEnd);
        SelectionProxy selectionProxy = selector.execute();
        
        // Build a cache of errors
        std::vector<int> selection(nFramesPlus1Sq, -1);
        std::vector<float> errors(nFrames + 1, -1.0f);
        errors[2] = analysis.get_error_by_frame(selStart, selEnd);
        for (int i = 3; i < nFrames + 1; i++) {
            errors[i] = analysis.get_error_for_frames(selectionProxy.getSelection(i));
        }

        // Write to handles
        MDataHandle mSelectionHandle = data.outputValue(oaSelection);
        MFnIntArrayData intArrayData;
        mSelectionHandle.set(intArrayData.create(MIntArray(selectionProxy.getSelectionCache().data(), nFramesPlus1Sq)));
        mSelectionHandle.setClean();

        MDataHandle mSelectionErrorsHandle = data.outputValue(oaSelectionErrors);
        MFnFloatArrayData floatArrayData;
        mSelectionErrorsHandle.set(floatArrayData.create(MFloatArray(errors.data(), nFrames + 1)));
        mSelectionErrorsHandle.setClean();
        return MS::kSuccess;
        
    } else {
        return MS::kUnknownParameter;
    }
}