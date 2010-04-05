# This file was automatically generated by SWIG (http://www.swig.org).
# Version 1.3.36
#
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _tinkercell
import new
new_instancemethod = new.instancemethod
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


class ArrayOfStrings(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, ArrayOfStrings, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, ArrayOfStrings, name)
    __repr__ = _swig_repr
    __swig_setmethods__["length"] = _tinkercell.ArrayOfStrings_length_set
    __swig_getmethods__["length"] = _tinkercell.ArrayOfStrings_length_get
    if _newclass:length = _swig_property(_tinkercell.ArrayOfStrings_length_get, _tinkercell.ArrayOfStrings_length_set)
    __swig_setmethods__["strings"] = _tinkercell.ArrayOfStrings_strings_set
    __swig_getmethods__["strings"] = _tinkercell.ArrayOfStrings_strings_get
    if _newclass:strings = _swig_property(_tinkercell.ArrayOfStrings_strings_get, _tinkercell.ArrayOfStrings_strings_set)
    def __init__(self, *args): 
        this = _tinkercell.new_ArrayOfStrings(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _tinkercell.delete_ArrayOfStrings
    __del__ = lambda self : None;
ArrayOfStrings_swigregister = _tinkercell.ArrayOfStrings_swigregister
ArrayOfStrings_swigregister(ArrayOfStrings)

class ArrayOfItems(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, ArrayOfItems, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, ArrayOfItems, name)
    __repr__ = _swig_repr
    __swig_setmethods__["length"] = _tinkercell.ArrayOfItems_length_set
    __swig_getmethods__["length"] = _tinkercell.ArrayOfItems_length_get
    if _newclass:length = _swig_property(_tinkercell.ArrayOfItems_length_get, _tinkercell.ArrayOfItems_length_set)
    __swig_setmethods__["items"] = _tinkercell.ArrayOfItems_items_set
    __swig_getmethods__["items"] = _tinkercell.ArrayOfItems_items_get
    if _newclass:items = _swig_property(_tinkercell.ArrayOfItems_items_get, _tinkercell.ArrayOfItems_items_set)
    def __init__(self, *args): 
        this = _tinkercell.new_ArrayOfItems(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _tinkercell.delete_ArrayOfItems
    __del__ = lambda self : None;
ArrayOfItems_swigregister = _tinkercell.ArrayOfItems_swigregister
ArrayOfItems_swigregister(ArrayOfItems)

class Matrix(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Matrix, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Matrix, name)
    __repr__ = _swig_repr
    __swig_setmethods__["rows"] = _tinkercell.Matrix_rows_set
    __swig_getmethods__["rows"] = _tinkercell.Matrix_rows_get
    if _newclass:rows = _swig_property(_tinkercell.Matrix_rows_get, _tinkercell.Matrix_rows_set)
    __swig_setmethods__["cols"] = _tinkercell.Matrix_cols_set
    __swig_getmethods__["cols"] = _tinkercell.Matrix_cols_get
    if _newclass:cols = _swig_property(_tinkercell.Matrix_cols_get, _tinkercell.Matrix_cols_set)
    __swig_setmethods__["values"] = _tinkercell.Matrix_values_set
    __swig_getmethods__["values"] = _tinkercell.Matrix_values_get
    if _newclass:values = _swig_property(_tinkercell.Matrix_values_get, _tinkercell.Matrix_values_set)
    __swig_setmethods__["rownames"] = _tinkercell.Matrix_rownames_set
    __swig_getmethods__["rownames"] = _tinkercell.Matrix_rownames_get
    if _newclass:rownames = _swig_property(_tinkercell.Matrix_rownames_get, _tinkercell.Matrix_rownames_set)
    __swig_setmethods__["colnames"] = _tinkercell.Matrix_colnames_set
    __swig_getmethods__["colnames"] = _tinkercell.Matrix_colnames_get
    if _newclass:colnames = _swig_property(_tinkercell.Matrix_colnames_get, _tinkercell.Matrix_colnames_set)
    def __init__(self, *args): 
        this = _tinkercell.new_Matrix(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _tinkercell.delete_Matrix
    __del__ = lambda self : None;
Matrix_swigregister = _tinkercell.Matrix_swigregister
Matrix_swigregister(Matrix)

class TableOfStrings(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, TableOfStrings, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, TableOfStrings, name)
    __repr__ = _swig_repr
    __swig_setmethods__["rows"] = _tinkercell.TableOfStrings_rows_set
    __swig_getmethods__["rows"] = _tinkercell.TableOfStrings_rows_get
    if _newclass:rows = _swig_property(_tinkercell.TableOfStrings_rows_get, _tinkercell.TableOfStrings_rows_set)
    __swig_setmethods__["cols"] = _tinkercell.TableOfStrings_cols_set
    __swig_getmethods__["cols"] = _tinkercell.TableOfStrings_cols_get
    if _newclass:cols = _swig_property(_tinkercell.TableOfStrings_cols_get, _tinkercell.TableOfStrings_cols_set)
    __swig_setmethods__["strings"] = _tinkercell.TableOfStrings_strings_set
    __swig_getmethods__["strings"] = _tinkercell.TableOfStrings_strings_get
    if _newclass:strings = _swig_property(_tinkercell.TableOfStrings_strings_get, _tinkercell.TableOfStrings_strings_set)
    __swig_setmethods__["rownames"] = _tinkercell.TableOfStrings_rownames_set
    __swig_getmethods__["rownames"] = _tinkercell.TableOfStrings_rownames_get
    if _newclass:rownames = _swig_property(_tinkercell.TableOfStrings_rownames_get, _tinkercell.TableOfStrings_rownames_set)
    __swig_setmethods__["colnames"] = _tinkercell.TableOfStrings_colnames_set
    __swig_getmethods__["colnames"] = _tinkercell.TableOfStrings_colnames_get
    if _newclass:colnames = _swig_property(_tinkercell.TableOfStrings_colnames_get, _tinkercell.TableOfStrings_colnames_set)
    def __init__(self, *args): 
        this = _tinkercell.new_TableOfStrings(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _tinkercell.delete_TableOfStrings
    __del__ = lambda self : None;
TableOfStrings_swigregister = _tinkercell.TableOfStrings_swigregister
TableOfStrings_swigregister(TableOfStrings)

newMatrix = _tinkercell.newMatrix
newTableOfStrings = _tinkercell.newTableOfStrings
newArrayOfStrings = _tinkercell.newArrayOfStrings
newArrayOfItems = _tinkercell.newArrayOfItems
getValue = _tinkercell.getValue
setValue = _tinkercell.setValue
getRowName = _tinkercell.getRowName
setRowName = _tinkercell.setRowName
getColumnName = _tinkercell.getColumnName
setColumnName = _tinkercell.setColumnName
getString = _tinkercell.getString
setString = _tinkercell.setString
nthString = _tinkercell.nthString
setNthString = _tinkercell.setNthString
nthItem = _tinkercell.nthItem
setNthItem = _tinkercell.setNthItem
deleteMatrix = _tinkercell.deleteMatrix
deleteTableOfStrings = _tinkercell.deleteTableOfStrings
deleteArrayOfItems = _tinkercell.deleteArrayOfItems
deleteArrayOfStrings = _tinkercell.deleteArrayOfStrings
cbind = _tinkercell.cbind
rbind = _tinkercell.rbind
tc_allItems = _tinkercell.tc_allItems
tc_selectedItems = _tinkercell.tc_selectedItems
tc_itemsOfFamily = _tinkercell.tc_itemsOfFamily
tc_itemsOfFamilyFrom = _tinkercell.tc_itemsOfFamilyFrom
tc_find = _tinkercell.tc_find
tc_findItems = _tinkercell.tc_findItems
tc_select = _tinkercell.tc_select
tc_deselect = _tinkercell.tc_deselect
tc_getName = _tinkercell.tc_getName
tc_rename = _tinkercell.tc_rename
tc_getNames = _tinkercell.tc_getNames
tc_getFamily = _tinkercell.tc_getFamily
tc_isA = _tinkercell.tc_isA
tc_print = _tinkercell.tc_print
tc_errorReport = _tinkercell.tc_errorReport
tc_printTable = _tinkercell.tc_printTable
tc_printFile = _tinkercell.tc_printFile
tc_clear = _tinkercell.tc_clear
tc_remove = _tinkercell.tc_remove
tc_getY = _tinkercell.tc_getY
tc_getX = _tinkercell.tc_getX
tc_getPos = _tinkercell.tc_getPos
tc_setPos = _tinkercell.tc_setPos
tc_setPosMulti = _tinkercell.tc_setPosMulti
tc_moveSelected = _tinkercell.tc_moveSelected
tc_isWindows = _tinkercell.tc_isWindows
tc_isMac = _tinkercell.tc_isMac
tc_isLinux = _tinkercell.tc_isLinux
tc_appDir = _tinkercell.tc_appDir
tc_createInputWindowFromFile = _tinkercell.tc_createInputWindowFromFile
tc_createInputWindow = _tinkercell.tc_createInputWindow
tc_addInputWindowOptions = _tinkercell.tc_addInputWindowOptions
tc_addInputWindowCheckbox = _tinkercell.tc_addInputWindowCheckbox
tc_openNewWindow = _tinkercell.tc_openNewWindow
tc_getChildren = _tinkercell.tc_getChildren
tc_getParent = _tinkercell.tc_getParent
tc_getNumericalData = _tinkercell.tc_getNumericalData
tc_setNumericalData = _tinkercell.tc_setNumericalData
tc_getTextData = _tinkercell.tc_getTextData
tc_setTextData = _tinkercell.tc_setTextData
tc_getNumericalDataNames = _tinkercell.tc_getNumericalDataNames
tc_getTextDataNames = _tinkercell.tc_getTextDataNames
tc_zoom = _tinkercell.tc_zoom
tc_getString = _tinkercell.tc_getString
tc_getFilename = _tinkercell.tc_getFilename
tc_getFromList = _tinkercell.tc_getFromList
tc_getNumber = _tinkercell.tc_getNumber
tc_getNumbers = _tinkercell.tc_getNumbers
tc_askQuestion = _tinkercell.tc_askQuestion
tc_messageDialog = _tinkercell.tc_messageDialog
tc_thisThread = _tinkercell.tc_thisThread
tc_createSliders = _tinkercell.tc_createSliders
tc_getColor = _tinkercell.tc_getColor
tc_setColor = _tinkercell.tc_setColor
tc_changeNodeImage = _tinkercell.tc_changeNodeImage
tc_changeArrowHead = _tinkercell.tc_changeArrowHead
tc_setSize = _tinkercell.tc_setSize
tc_getWidth = _tinkercell.tc_getWidth
tc_getHeight = _tinkercell.tc_getHeight
tc_setAngle = _tinkercell.tc_setAngle
tc_getAngle = _tinkercell.tc_getAngle
tc_Main_api_initialize = _tinkercell.tc_Main_api_initialize
tc_showProgress = _tinkercell.tc_showProgress
tc_CThread_api_initialize = _tinkercell.tc_CThread_api_initialize
tc_getParameters = _tinkercell.tc_getParameters
tc_getInitialValues = _tinkercell.tc_getInitialValues
tc_setInitialValues = _tinkercell.tc_setInitialValues
tc_getFixedVariables = _tinkercell.tc_getFixedVariables
tc_getParametersAndFixedVariables = _tinkercell.tc_getParametersAndFixedVariables
tc_getTextAttribute = _tinkercell.tc_getTextAttribute
tc_getParameter = _tinkercell.tc_getParameter
tc_getParametersNamed = _tinkercell.tc_getParametersNamed
tc_getParametersExcept = _tinkercell.tc_getParametersExcept
tc_getAllTextNamed = _tinkercell.tc_getAllTextNamed
tc_setTextAttribute = _tinkercell.tc_setTextAttribute
tc_setParameter = _tinkercell.tc_setParameter
tc_BasicInformationTool_Text_api = _tinkercell.tc_BasicInformationTool_Text_api
tc_BasicInformationTool_Numeric_api = _tinkercell.tc_BasicInformationTool_Numeric_api
tc_insertConnection = _tinkercell.tc_insertConnection
tc_getConnectedNodes = _tinkercell.tc_getConnectedNodes
tc_getConnectedNodesIn = _tinkercell.tc_getConnectedNodesIn
tc_getConnectedNodesOut = _tinkercell.tc_getConnectedNodesOut
tc_getConnectedNodesOther = _tinkercell.tc_getConnectedNodesOther
tc_getConnections = _tinkercell.tc_getConnections
tc_getConnectionsIn = _tinkercell.tc_getConnectionsIn
tc_getConnectionsOut = _tinkercell.tc_getConnectionsOut
tc_getConnectionsOther = _tinkercell.tc_getConnectionsOther
tc_ConnectionInsertion_api = _tinkercell.tc_ConnectionInsertion_api
tc_getControlPointX = _tinkercell.tc_getControlPointX
tc_getControlPointY = _tinkercell.tc_getControlPointY
tc_setControlPoint = _tinkercell.tc_setControlPoint
tc_setCenterPoint = _tinkercell.tc_setCenterPoint
tc_getCenterPointX = _tinkercell.tc_getCenterPointX
tc_getCenterPointY = _tinkercell.tc_getCenterPointY
tc_setStraight = _tinkercell.tc_setStraight
tc_setAllStraight = _tinkercell.tc_setAllStraight
tc_setLineWidth = _tinkercell.tc_setLineWidth
tc_ConnectionSelection_api = _tinkercell.tc_ConnectionSelection_api
tc_merge = _tinkercell.tc_merge
tc_separate = _tinkercell.tc_separate
tc_GroupHandlerTool_api = _tinkercell.tc_GroupHandlerTool_api
tc_insert = _tinkercell.tc_insert
tc_NodeInsertion_api = _tinkercell.tc_NodeInsertion_api
tc_getStoichiometry = _tinkercell.tc_getStoichiometry
tc_setStoichiometry = _tinkercell.tc_setStoichiometry
tc_getRates = _tinkercell.tc_getRates
tc_setRates = _tinkercell.tc_setRates
tc_getStoichiometryFor = _tinkercell.tc_getStoichiometryFor
tc_getRate = _tinkercell.tc_getRate
tc_setRate = _tinkercell.tc_setRate
tc_setStoichiometryFor = _tinkercell.tc_setStoichiometryFor
tc_StoichiometryTool_api = _tinkercell.tc_StoichiometryTool_api
tc_compileAndRun = _tinkercell.tc_compileAndRun
tc_compileBuildLoad = _tinkercell.tc_compileBuildLoad
tc_compileBuildLoadSliders = _tinkercell.tc_compileBuildLoadSliders
tc_runPythonCode = _tinkercell.tc_runPythonCode
tc_runPythonFile = _tinkercell.tc_runPythonFile
tc_addPythonPlugin = _tinkercell.tc_addPythonPlugin
tc_callFunction = _tinkercell.tc_callFunction
tc_loadLibrary = _tinkercell.tc_loadLibrary
tc_addFunction = _tinkercell.tc_addFunction
tc_callback = _tinkercell.tc_callback
tc_callWhenExiting = _tinkercell.tc_callWhenExiting
tc_DynamicLibraryMenu_api = _tinkercell.tc_DynamicLibraryMenu_api
tc_LoadCLibraries_api = _tinkercell.tc_LoadCLibraries_api
tc_PythonTool_api = _tinkercell.tc_PythonTool_api
tc_surface = _tinkercell.tc_surface
tc_plot = _tinkercell.tc_plot
tc_scatterplot = _tinkercell.tc_scatterplot
tc_errorBars = _tinkercell.tc_errorBars
tc_hist = _tinkercell.tc_hist
tc_multiplot = _tinkercell.tc_multiplot
tc_getPlotData = _tinkercell.tc_getPlotData
tc_PlotTool_api = _tinkercell.tc_PlotTool_api
tc_getModelParameters = _tinkercell.tc_getModelParameters
tc_writeModel = _tinkercell.tc_writeModel
tc_ModelFileGenerator_api = _tinkercell.tc_ModelFileGenerator_api
tc_getEventTriggers = _tinkercell.tc_getEventTriggers
tc_getEventResponses = _tinkercell.tc_getEventResponses
tc_addEvent = _tinkercell.tc_addEvent
tc_SimulationEventsTool_api = _tinkercell.tc_SimulationEventsTool_api
tc_getForcingFunctionNames = _tinkercell.tc_getForcingFunctionNames
tc_getForcingFunctionAssignments = _tinkercell.tc_getForcingFunctionAssignments
tc_addForcingFunction = _tinkercell.tc_addForcingFunction
tc_AssignmentFunctionsTool_api = _tinkercell.tc_AssignmentFunctionsTool_api
tc_displayText = _tinkercell.tc_displayText
tc_displayNumber = _tinkercell.tc_displayNumber
tc_setDisplayLabelColor = _tinkercell.tc_setDisplayLabelColor
tc_highlight = _tinkercell.tc_highlight
tc_CLabelsTool_api = _tinkercell.tc_CLabelsTool_api
tc_getAnnotation = _tinkercell.tc_getAnnotation
tc_setAnnotation = _tinkercell.tc_setAnnotation
tc_NameFamily_api_initialize = _tinkercell.tc_NameFamily_api_initialize
tc_partsIn = _tinkercell.tc_partsIn
tc_partsUpstream = _tinkercell.tc_partsUpstream
tc_partsDownstream = _tinkercell.tc_partsDownstream
tc_alignParts = _tinkercell.tc_alignParts
tc_AutoGeneRegulatoryTool_api = _tinkercell.tc_AutoGeneRegulatoryTool_api
tc_loadSBMLString = _tinkercell.tc_loadSBMLString
tc_loadAntimonyString = _tinkercell.tc_loadAntimonyString
tc_loadSBMLFile = _tinkercell.tc_loadSBMLFile
tc_loadAntimonyFile = _tinkercell.tc_loadAntimonyFile
tc_getSBMLString = _tinkercell.tc_getSBMLString
tc_getAntimonyString = _tinkercell.tc_getAntimonyString
tc_writeSBMLFile = _tinkercell.tc_writeSBMLFile
tc_writeAntimonyFile = _tinkercell.tc_writeAntimonyFile
tc_Antimony_api = _tinkercell.tc_Antimony_api


def toItems(array):
    n = len(array);
    A = newArrayOfItems(n);
    for i in range(0, n):
        setNthItem(A, i, array[i]);

    return A;

def fromItems(array):
    n = array.length;
    array = range(0,n);
    for i in range(0, n):
        A[i] = nthItem(array,i);

    deleteArrayOfItems(array);
    return A;

def toStrings(array):
    n = len(array);
    A = newArrayOfStrings(n);
    for i in range(0, n):
        setNthString(A, i, array[i]);

    return A;

def fromStrings(array):
    n = array.length;
    A = range(0,n);
    for i in range(0, n):
        A[i] = nthString(array,i);

    deleteArrayOfStrings(array);
    return A;

def fromMatrix(matrix, row_wise = False):
    n = matrix.rows;
    m = matrix.cols;
    cols = fromStrings(matrix.colnames);
    rows = fromStrings(matrix.rownames);
    if row_wise:
        A = range(0,n);
        for i in range(0, n):
            A[i] = range(0,m);
            for j in range(0,m):
                A[i][j] = getValue(matrix,i,j);
    else:
        A = range(0,m);
        for i in range(0, m):
            A[i] = range(0,n);
            for j in range(0,n):
                A[i][j] = getValue(matrix,j,i);

    deleteMatrix(matrix);
    return [rows, cols, A];

def toMatrix(lists, row_wise = False , rows = [], cols = []):
    n = len(lists);
    m = len(lists[0]);
    if row_wise:
        A = newMatrix(n,m);
    else:
        A = newMatrix(m,n);
    for i in range(0, n):
        for j in range(0,m):
            if row_wise:
                setNthValue(matrix,i,j,lists[i][j]);
            else:
                setNthValue(matrix,j,i,lists[i][j]);
    n = len(rows);
    m = len(cols);

    for i in range(0,n):
        setRowName(matrix,i,rows[i]);

    for i in range(0,m):
        setColName(matrix,i,cols[i]);

    return A;