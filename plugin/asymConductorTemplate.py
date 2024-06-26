import maya.mel
import maya.cmds as cmds
import mtoa.ui.ae.utils as aeUtils
from mtoa.ui.ae.shaderTemplate import ShaderAETemplate

class AEasymConductorTemplate(ShaderAETemplate):
    def setup(self):
        self.addSwatch()
        self.beginScrollLayout()
        self.addCustom('message', 'AEshaderTypeNew', 'AEshaderTypeReplace')

        self.addControl('boundary_depth', label='Boundary Depth z')
        self.addControl('alpha_x_a', label='Roughness Layer A (X)')
        self.addControl('alpha_y_a', label='Roughness Layer A (Y)')
        self.addControl('alpha_x_b', label='Roughness Layer B (X)')
        self.addControl('alpha_y_b', label='Roughness Layer B (Y)')
        self.addControl('albedoA', label='AlbedoA')
        self.addControl('albedoB', label='AlbedoB')
        self.addControl('bd_eval', label='BDEval')

        maya.mel.eval('AEdependNodeTemplate '+self.nodeName)
        self.addExtraControls()
        self.endScrollLayout()