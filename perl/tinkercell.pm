# This file was automatically generated by SWIG (http://www.swig.org).
# Version 1.3.40
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.

package tinkercell;
use base qw(Exporter);
use base qw(DynaLoader);
package tinkercellc;
bootstrap tinkercell;
package tinkercell;
@EXPORT = qw();

# ---------- BASE METHODS -------------

package tinkercell;

sub TIEHASH {
    my ($classname,$obj) = @_;
    return bless $obj, $classname;
}

sub CLEAR { }

sub FIRSTKEY { }

sub NEXTKEY { }

sub FETCH {
    my ($self,$field) = @_;
    my $member_func = "swig_${field}_get";
    $self->$member_func();
}

sub STORE {
    my ($self,$field,$newval) = @_;
    my $member_func = "swig_${field}_set";
    $self->$member_func($newval);
}

sub this {
    my $ptr = shift;
    return tied(%$ptr);
}


# ------- FUNCTION WRAPPERS --------

package tinkercell;

*tc_createMatrix = *tinkercellc::tc_createMatrix;
*tc_createTable = *tinkercellc::tc_createTable;
*tc_createStringsArray = *tinkercellc::tc_createStringsArray;
*tc_createItemsArray = *tinkercellc::tc_createItemsArray;
*tc_getMatrixValue = *tinkercellc::tc_getMatrixValue;
*tc_setMatrixValue = *tinkercellc::tc_setMatrixValue;
*tc_getRowName = *tinkercellc::tc_getRowName;
*tc_setRowName = *tinkercellc::tc_setRowName;
*tc_getColumnName = *tinkercellc::tc_getColumnName;
*tc_setColumnName = *tinkercellc::tc_setColumnName;
*tc_getTableValue = *tinkercellc::tc_getTableValue;
*tc_setTableValue = *tinkercellc::tc_setTableValue;
*tc_getString = *tinkercellc::tc_getString;
*tc_setString = *tinkercellc::tc_setString;
*tc_getItem = *tinkercellc::tc_getItem;
*tc_setItem = *tinkercellc::tc_setItem;
*tc_getStringIndex = *tinkercellc::tc_getStringIndex;
*tc_getRowIndex = *tinkercellc::tc_getRowIndex;
*tc_getColumnIndex = *tinkercellc::tc_getColumnIndex;
*tc_deleteMatrix = *tinkercellc::tc_deleteMatrix;
*tc_deleteTable = *tinkercellc::tc_deleteTable;
*tc_deleteItemsArray = *tinkercellc::tc_deleteItemsArray;
*tc_deleteStringsArray = *tinkercellc::tc_deleteStringsArray;
*tc_appendColumns = *tinkercellc::tc_appendColumns;
*tc_appendRows = *tinkercellc::tc_appendRows;
*tc_printMatrixToFile = *tinkercellc::tc_printMatrixToFile;
*tc_printOutMatrix = *tinkercellc::tc_printOutMatrix;
*tc_printTableToFile = *tinkercellc::tc_printTableToFile;
*tc_printOutTable = *tinkercellc::tc_printOutTable;
*tc_allItems = *tinkercellc::tc_allItems;
*tc_selectedItems = *tinkercellc::tc_selectedItems;
*tc_itemsOfFamily = *tinkercellc::tc_itemsOfFamily;
*tc_itemsOfFamilyFrom = *tinkercellc::tc_itemsOfFamilyFrom;
*tc_find = *tinkercellc::tc_find;
*tc_findItems = *tinkercellc::tc_findItems;
*tc_findItemsUsingRegexp = *tinkercellc::tc_findItemsUsingRegexp;
*tc_select = *tinkercellc::tc_select;
*tc_deselect = *tinkercellc::tc_deselect;
*tc_getName = *tinkercellc::tc_getName;
*tc_getUniqueName = *tinkercellc::tc_getUniqueName;
*tc_rename = *tinkercellc::tc_rename;
*tc_getNames = *tinkercellc::tc_getNames;
*tc_getUniqueNames = *tinkercellc::tc_getUniqueNames;
*tc_getFamily = *tinkercellc::tc_getFamily;
*tc_isA = *tinkercellc::tc_isA;
*tc_print = *tinkercellc::tc_print;
*tc_openUrl = *tinkercellc::tc_openUrl;
*tc_errorReport = *tinkercellc::tc_errorReport;
*tc_printMatrix = *tinkercellc::tc_printMatrix;
*tc_printFile = *tinkercellc::tc_printFile;
*tc_clear = *tinkercellc::tc_clear;
*tc_remove = *tinkercellc::tc_remove;
*tc_insert = *tinkercellc::tc_insert;
*tc_insertConnection = *tinkercellc::tc_insertConnection;
*tc_getConnectedNodes = *tinkercellc::tc_getConnectedNodes;
*tc_getConnections = *tinkercellc::tc_getConnections;
*tc_getY = *tinkercellc::tc_getY;
*tc_getX = *tinkercellc::tc_getX;
*tc_getPos = *tinkercellc::tc_getPos;
*tc_setPos = *tinkercellc::tc_setPos;
*tc_setPosMulti = *tinkercellc::tc_setPosMulti;
*tc_moveSelected = *tinkercellc::tc_moveSelected;
*tc_isWindows = *tinkercellc::tc_isWindows;
*tc_isMac = *tinkercellc::tc_isMac;
*tc_isLinux = *tinkercellc::tc_isLinux;
*tc_appDir = *tinkercellc::tc_appDir;
*tc_homeDir = *tinkercellc::tc_homeDir;
*tc_createInputWindowForScript = *tinkercellc::tc_createInputWindowForScript;
*tc_createInputWindow = *tinkercellc::tc_createInputWindow;
*tc_addInputWindowOptions = *tinkercellc::tc_addInputWindowOptions;
*tc_addInputWindowCheckbox = *tinkercellc::tc_addInputWindowCheckbox;
*tc_openNewWindow = *tinkercellc::tc_openNewWindow;
*tc_getChildren = *tinkercellc::tc_getChildren;
*tc_getParent = *tinkercellc::tc_getParent;
*tc_getNumericalData = *tinkercellc::tc_getNumericalData;
*tc_setNumericalData = *tinkercellc::tc_setNumericalData;
*tc_setNumericalValues = *tinkercellc::tc_setNumericalValues;
*tc_setNumericalValue = *tinkercellc::tc_setNumericalValue;
*tc_getTextData = *tinkercellc::tc_getTextData;
*tc_setTextData = *tinkercellc::tc_setTextData;
*tc_setTextValues = *tinkercellc::tc_setTextValues;
*tc_getNumericalValue = *tinkercellc::tc_getNumericalValue;
*tc_getNumericalValueUsingRegexp = *tinkercellc::tc_getNumericalValueUsingRegexp;
*tc_getTextValue = *tinkercellc::tc_getTextValue;
*tc_getTextValueUsingRegexp = *tinkercellc::tc_getTextValueUsingRegexp;
*tc_setTextValue = *tinkercellc::tc_setTextValue;
*tc_getNumericalDataNames = *tinkercellc::tc_getNumericalDataNames;
*tc_getTextDataNames = *tinkercellc::tc_getTextDataNames;
*tc_zoom = *tinkercellc::tc_zoom;
*tc_viewWindow = *tinkercellc::tc_viewWindow;
*tc_getStringDialog = *tinkercellc::tc_getStringDialog;
*tc_getFilename = *tinkercellc::tc_getFilename;
*tc_getStringFromList = *tinkercellc::tc_getStringFromList;
*tc_getNumber = *tinkercellc::tc_getNumber;
*tc_getNumbers = *tinkercellc::tc_getNumbers;
*tc_askQuestion = *tinkercellc::tc_askQuestion;
*tc_messageDialog = *tinkercellc::tc_messageDialog;
*tc_openFile = *tinkercellc::tc_openFile;
*tc_saveToFile = *tinkercellc::tc_saveToFile;
*tc_thisThread = *tinkercellc::tc_thisThread;
*tc_createSliders = *tinkercellc::tc_createSliders;
*tc_getColor = *tinkercellc::tc_getColor;
*tc_setColor = *tinkercellc::tc_setColor;
*tc_changeNodeImage = *tinkercellc::tc_changeNodeImage;
*tc_changeArrowHead = *tinkercellc::tc_changeArrowHead;
*tc_setSize = *tinkercellc::tc_setSize;
*tc_getWidth = *tinkercellc::tc_getWidth;
*tc_getHeight = *tinkercellc::tc_getHeight;
*tc_rotate = *tinkercellc::tc_rotate;
*tc_screenshot = *tinkercellc::tc_screenshot;
*tc_screenWidth = *tinkercellc::tc_screenWidth;
*tc_screenHeight = *tinkercellc::tc_screenHeight;
*tc_screenX = *tinkercellc::tc_screenX;
*tc_screenY = *tinkercellc::tc_screenY;
*tc_annotations = *tinkercellc::tc_annotations;
*tc_insertAnnotations = *tinkercellc::tc_insertAnnotations;
*tc_getControlPointX = *tinkercellc::tc_getControlPointX;
*tc_getControlPointY = *tinkercellc::tc_getControlPointY;
*tc_setControlPoint = *tinkercellc::tc_setControlPoint;
*tc_setCenterPoint = *tinkercellc::tc_setCenterPoint;
*tc_getCenterPointX = *tinkercellc::tc_getCenterPointX;
*tc_getCenterPointY = *tinkercellc::tc_getCenterPointY;
*tc_setStraight = *tinkercellc::tc_setStraight;
*tc_setAllStraight = *tinkercellc::tc_setAllStraight;
*tc_setLineWidth = *tinkercellc::tc_setLineWidth;
*tc_Main_api_initialize = *tinkercellc::tc_Main_api_initialize;
*tc_showProgress = *tinkercellc::tc_showProgress;
*tc_callback = *tinkercellc::tc_callback;
*tc_callWhenExiting = *tinkercellc::tc_callWhenExiting;
*tc_CThread_api_initialize = *tinkercellc::tc_CThread_api_initialize;
*tc_displayText = *tinkercellc::tc_displayText;
*tc_displayNumber = *tinkercellc::tc_displayNumber;
*tc_setDisplayLabelColor = *tinkercellc::tc_setDisplayLabelColor;
*tc_highlight = *tinkercellc::tc_highlight;
*tc_burn = *tinkercellc::tc_burn;
*tc_setAlpha = *tinkercellc::tc_setAlpha;
*tc_LabelingTool_api = *tinkercellc::tc_LabelingTool_api;
*ApplySpringForce = *tinkercellc::ApplySpringForce;
*tc_getParameters = *tinkercellc::tc_getParameters;
*tc_getInitialValues = *tinkercellc::tc_getInitialValues;
*tc_setInitialValues = *tinkercellc::tc_setInitialValues;
*tc_getFixedVariables = *tinkercellc::tc_getFixedVariables;
*tc_getParametersAndFixedVariables = *tinkercellc::tc_getParametersAndFixedVariables;
*tc_getTextAttribute = *tinkercellc::tc_getTextAttribute;
*tc_getParameter = *tinkercellc::tc_getParameter;
*tc_getParametersNamed = *tinkercellc::tc_getParametersNamed;
*tc_getParametersExcept = *tinkercellc::tc_getParametersExcept;
*tc_getAllTextNamed = *tinkercellc::tc_getAllTextNamed;
*tc_setTextAttribute = *tinkercellc::tc_setTextAttribute;
*tc_setParameter = *tinkercellc::tc_setParameter;
*tc_setTextAttributeByName = *tinkercellc::tc_setTextAttributeByName;
*tc_setParameterByName = *tinkercellc::tc_setParameterByName;
*tc_setTextAttributes = *tinkercellc::tc_setTextAttributes;
*tc_setParameters = *tinkercellc::tc_setParameters;
*tc_BasicInformationTool_Text_api = *tinkercellc::tc_BasicInformationTool_Text_api;
*tc_BasicInformationTool_Numeric_api = *tinkercellc::tc_BasicInformationTool_Numeric_api;
*tc_getConnectedNodesWithRole = *tinkercellc::tc_getConnectedNodesWithRole;
*tc_getConnectionsWithRole = *tinkercellc::tc_getConnectionsWithRole;
*tc_ConnectionInsertion_api = *tinkercellc::tc_ConnectionInsertion_api;
*tc_merge = *tinkercellc::tc_merge;
*tc_separate = *tinkercellc::tc_separate;
*tc_GroupHandlerTool_api = *tinkercellc::tc_GroupHandlerTool_api;
*tc_getStoichiometry = *tinkercellc::tc_getStoichiometry;
*tc_setStoichiometry = *tinkercellc::tc_setStoichiometry;
*tc_getRates = *tinkercellc::tc_getRates;
*tc_setRates = *tinkercellc::tc_setRates;
*tc_getStoichiometryFor = *tinkercellc::tc_getStoichiometryFor;
*tc_getRate = *tinkercellc::tc_getRate;
*tc_setRate = *tinkercellc::tc_setRate;
*tc_setStoichiometryFor = *tinkercellc::tc_setStoichiometryFor;
*tc_StoichiometryTool_api = *tinkercellc::tc_StoichiometryTool_api;
*tc_compileAndRun = *tinkercellc::tc_compileAndRun;
*tc_compileBuildLoad = *tinkercellc::tc_compileBuildLoad;
*tc_compileBuildLoadSliders = *tinkercellc::tc_compileBuildLoadSliders;
*tc_runPythonCode = *tinkercellc::tc_runPythonCode;
*tc_runPythonFile = *tinkercellc::tc_runPythonFile;
*tc_addPythonPlugin = *tinkercellc::tc_addPythonPlugin;
*tc_callFunction = *tinkercellc::tc_callFunction;
*tc_displayCode = *tinkercellc::tc_displayCode;
*tc_loadLibrary = *tinkercellc::tc_loadLibrary;
*tc_OctaveTool_api = *tinkercellc::tc_OctaveTool_api;
*tc_addFunction = *tinkercellc::tc_addFunction;
*tc_runOctaveCode = *tinkercellc::tc_runOctaveCode;
*tc_runOctaveFile = *tinkercellc::tc_runOctaveFile;
*tc_addOctavePlugin = *tinkercellc::tc_addOctavePlugin;
*tc_DynamicLibraryMenu_api = *tinkercellc::tc_DynamicLibraryMenu_api;
*tc_LoadCLibraries_api = *tinkercellc::tc_LoadCLibraries_api;
*tc_PythonTool_api = *tinkercellc::tc_PythonTool_api;
*tc_surface = *tinkercellc::tc_surface;
*tc_plot = *tinkercellc::tc_plot;
*tc_scatterplot = *tinkercellc::tc_scatterplot;
*tc_errorBars = *tinkercellc::tc_errorBars;
*tc_hist = *tinkercellc::tc_hist;
*tc_closePlots = *tinkercellc::tc_closePlots;
*tc_multiplot = *tinkercellc::tc_multiplot;
*tc_holdPlot = *tinkercellc::tc_holdPlot;
*tc_clusterPlots = *tinkercellc::tc_clusterPlots;
*tc_getPlotData = *tinkercellc::tc_getPlotData;
*tc_gnuplot = *tinkercellc::tc_gnuplot;
*tc_savePlot = *tinkercellc::tc_savePlot;
*tc_setLogScale = *tinkercellc::tc_setLogScale;
*tc_PlotTool_api = *tinkercellc::tc_PlotTool_api;
*tc_writeModel = *tinkercellc::tc_writeModel;
*tc_ModelFileGenerator_api = *tinkercellc::tc_ModelFileGenerator_api;
*tc_getEventTriggers = *tinkercellc::tc_getEventTriggers;
*tc_getEventResponses = *tinkercellc::tc_getEventResponses;
*tc_addEvent = *tinkercellc::tc_addEvent;
*tc_SimulationEventsTool_api = *tinkercellc::tc_SimulationEventsTool_api;
*tc_getForcingFunctionNames = *tinkercellc::tc_getForcingFunctionNames;
*tc_getForcingFunctionAssignments = *tinkercellc::tc_getForcingFunctionAssignments;
*tc_addForcingFunction = *tinkercellc::tc_addForcingFunction;
*tc_AssignmentFunctionsTool_api = *tinkercellc::tc_AssignmentFunctionsTool_api;
*tc_partsIn = *tinkercellc::tc_partsIn;
*tc_partsUpstream = *tinkercellc::tc_partsUpstream;
*tc_partsDownstream = *tinkercellc::tc_partsDownstream;
*tc_alignParts = *tinkercellc::tc_alignParts;
*tc_alignPartsOnPlasmid = *tinkercellc::tc_alignPartsOnPlasmid;
*tc_setSequence = *tinkercellc::tc_setSequence;
*tc_AutoGeneRegulatoryTool_api = *tinkercellc::tc_AutoGeneRegulatoryTool_api;
*tc_exportSBML = *tinkercellc::tc_exportSBML;
*tc_getSBMLString = *tinkercellc::tc_getSBMLString;
*tc_getAntimonyString = *tinkercellc::tc_getAntimonyString;
*tc_importSBML = *tinkercellc::tc_importSBML;
*tc_exportAntimony = *tinkercellc::tc_exportAntimony;
*tc_importAntimony = *tinkercellc::tc_importAntimony;
*tc_exportMatlab = *tinkercellc::tc_exportMatlab;
*tc_SBML_api = *tinkercellc::tc_SBML_api;
*tc_simulateDeterministic = *tinkercellc::tc_simulateDeterministic;
*tc_simulateStochastic = *tinkercellc::tc_simulateStochastic;
*tc_simulateHybrid = *tinkercellc::tc_simulateHybrid;
*tc_simulateTauLeap = *tinkercellc::tc_simulateTauLeap;
*tc_getSteadyState = *tinkercellc::tc_getSteadyState;
*tc_steadyStateScan = *tinkercellc::tc_steadyStateScan;
*tc_steadyStateScan2D = *tinkercellc::tc_steadyStateScan2D;
*tc_getJacobian = *tinkercellc::tc_getJacobian;
*tc_getEigenvalues = *tinkercellc::tc_getEigenvalues;
*tc_getUnscaledElasticities = *tinkercellc::tc_getUnscaledElasticities;
*tc_getUnscaledConcentrationCC = *tinkercellc::tc_getUnscaledConcentrationCC;
*tc_getUnscaledFluxCC = *tinkercellc::tc_getUnscaledFluxCC;
*tc_getScaledElasticities = *tinkercellc::tc_getScaledElasticities;
*tc_getScaledConcentrationCC = *tinkercellc::tc_getScaledConcentrationCC;
*tc_getScaledFluxCC = *tinkercellc::tc_getScaledFluxCC;
*tc_reducedStoichiometry = *tinkercellc::tc_reducedStoichiometry;
*tc_elementaryFluxModes = *tinkercellc::tc_elementaryFluxModes;
*tc_LMatrix = *tinkercellc::tc_LMatrix;
*tc_KMatrix = *tinkercellc::tc_KMatrix;
*tc_updateParameters = *tinkercellc::tc_updateParameters;
*tc_updateParameter = *tinkercellc::tc_updateParameter;
*tc_optimize = *tinkercellc::tc_optimize;
*tc_enableAssignmentRulesReordering = *tinkercellc::tc_enableAssignmentRulesReordering;
*tc_COPASI_api = *tinkercellc::tc_COPASI_api;
*tc_substituteModel = *tinkercellc::tc_substituteModel;
*tc_substituteEmptyModel = *tinkercellc::tc_substituteEmptyModel;
*tc_substituteOriginalModel = *tinkercellc::tc_substituteOriginalModel;
*tc_listOfPossibleModels = *tinkercellc::tc_listOfPossibleModels;
*tc_ModuleTool_api = *tinkercellc::tc_ModuleTool_api;

############# Class : tinkercell::tc_strings ##############

package tinkercell::tc_strings;
use vars qw(@ISA %OWNER %ITERATORS %BLESSEDMEMBERS);
@ISA = qw( tinkercell );
%OWNER = ();
%ITERATORS = ();
*swig_length_get = *tinkercellc::tc_strings_length_get;
*swig_length_set = *tinkercellc::tc_strings_length_set;
*swig_strings_get = *tinkercellc::tc_strings_strings_get;
*swig_strings_set = *tinkercellc::tc_strings_strings_set;
sub new {
    my $pkg = shift;
    my $self = tinkercellc::new_tc_strings(@_);
    bless $self, $pkg if defined($self);
}

sub DESTROY {
    return unless $_[0]->isa('HASH');
    my $self = tied(%{$_[0]});
    return unless defined $self;
    delete $ITERATORS{$self};
    if (exists $OWNER{$self}) {
        tinkercellc::delete_tc_strings($self);
        delete $OWNER{$self};
    }
}

sub DISOWN {
    my $self = shift;
    my $ptr = tied(%$self);
    delete $OWNER{$ptr};
}

sub ACQUIRE {
    my $self = shift;
    my $ptr = tied(%$self);
    $OWNER{$ptr} = 1;
}


############# Class : tinkercell::tc_items ##############

package tinkercell::tc_items;
use vars qw(@ISA %OWNER %ITERATORS %BLESSEDMEMBERS);
@ISA = qw( tinkercell );
%OWNER = ();
%ITERATORS = ();
*swig_length_get = *tinkercellc::tc_items_length_get;
*swig_length_set = *tinkercellc::tc_items_length_set;
*swig_items_get = *tinkercellc::tc_items_items_get;
*swig_items_set = *tinkercellc::tc_items_items_set;
sub new {
    my $pkg = shift;
    my $self = tinkercellc::new_tc_items(@_);
    bless $self, $pkg if defined($self);
}

sub DESTROY {
    return unless $_[0]->isa('HASH');
    my $self = tied(%{$_[0]});
    return unless defined $self;
    delete $ITERATORS{$self};
    if (exists $OWNER{$self}) {
        tinkercellc::delete_tc_items($self);
        delete $OWNER{$self};
    }
}

sub DISOWN {
    my $self = shift;
    my $ptr = tied(%$self);
    delete $OWNER{$ptr};
}

sub ACQUIRE {
    my $self = shift;
    my $ptr = tied(%$self);
    $OWNER{$ptr} = 1;
}


############# Class : tinkercell::tc_matrix ##############

package tinkercell::tc_matrix;
use vars qw(@ISA %OWNER %ITERATORS %BLESSEDMEMBERS);
@ISA = qw( tinkercell );
%OWNER = ();
%ITERATORS = ();
*swig_rows_get = *tinkercellc::tc_matrix_rows_get;
*swig_rows_set = *tinkercellc::tc_matrix_rows_set;
*swig_cols_get = *tinkercellc::tc_matrix_cols_get;
*swig_cols_set = *tinkercellc::tc_matrix_cols_set;
*swig_values_get = *tinkercellc::tc_matrix_values_get;
*swig_values_set = *tinkercellc::tc_matrix_values_set;
*swig_rownames_get = *tinkercellc::tc_matrix_rownames_get;
*swig_rownames_set = *tinkercellc::tc_matrix_rownames_set;
*swig_colnames_get = *tinkercellc::tc_matrix_colnames_get;
*swig_colnames_set = *tinkercellc::tc_matrix_colnames_set;
sub new {
    my $pkg = shift;
    my $self = tinkercellc::new_tc_matrix(@_);
    bless $self, $pkg if defined($self);
}

sub DESTROY {
    return unless $_[0]->isa('HASH');
    my $self = tied(%{$_[0]});
    return unless defined $self;
    delete $ITERATORS{$self};
    if (exists $OWNER{$self}) {
        tinkercellc::delete_tc_matrix($self);
        delete $OWNER{$self};
    }
}

sub DISOWN {
    my $self = shift;
    my $ptr = tied(%$self);
    delete $OWNER{$ptr};
}

sub ACQUIRE {
    my $self = shift;
    my $ptr = tied(%$self);
    $OWNER{$ptr} = 1;
}


############# Class : tinkercell::tc_table ##############

package tinkercell::tc_table;
use vars qw(@ISA %OWNER %ITERATORS %BLESSEDMEMBERS);
@ISA = qw( tinkercell );
%OWNER = ();
%ITERATORS = ();
*swig_rows_get = *tinkercellc::tc_table_rows_get;
*swig_rows_set = *tinkercellc::tc_table_rows_set;
*swig_cols_get = *tinkercellc::tc_table_cols_get;
*swig_cols_set = *tinkercellc::tc_table_cols_set;
*swig_strings_get = *tinkercellc::tc_table_strings_get;
*swig_strings_set = *tinkercellc::tc_table_strings_set;
*swig_rownames_get = *tinkercellc::tc_table_rownames_get;
*swig_rownames_set = *tinkercellc::tc_table_rownames_set;
*swig_colnames_get = *tinkercellc::tc_table_colnames_get;
*swig_colnames_set = *tinkercellc::tc_table_colnames_set;
sub new {
    my $pkg = shift;
    my $self = tinkercellc::new_tc_table(@_);
    bless $self, $pkg if defined($self);
}

sub DESTROY {
    return unless $_[0]->isa('HASH');
    my $self = tied(%{$_[0]});
    return unless defined $self;
    delete $ITERATORS{$self};
    if (exists $OWNER{$self}) {
        tinkercellc::delete_tc_table($self);
        delete $OWNER{$self};
    }
}

sub DISOWN {
    my $self = shift;
    my $ptr = tied(%$self);
    delete $OWNER{$ptr};
}

sub ACQUIRE {
    my $self = shift;
    my $ptr = tied(%$self);
    $OWNER{$ptr} = 1;
}


# ------- VARIABLE STUBS --------

package tinkercell;

1;
