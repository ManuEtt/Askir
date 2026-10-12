"""Exercise real exported GLB skin buffers and deliberate corruptions; no Unreal runtime claim."""
import importlib.util
from pathlib import Path
import struct
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
SPEC=importlib.util.spec_from_file_location("askir_source_glb_validation",ROOT/"Tools/validate_animated_sources_v78.py")
VALIDATOR=importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VALIDATOR)


class AnimatedSourceTests(unittest.TestCase):
    def source(self):
        return VALIDATOR.GLB(ROOT/"ArtSource/Animated_V78/Askir_Hero.glb")

    def overwrite(self,glb,index,format,value):
        accessor=glb.data["accessors"][index]
        view=glb.data["bufferViews"][accessor["bufferView"]]
        binary=bytearray(glb.binary)
        offset=view.get("byteOffset",0)+accessor.get("byteOffset",0)
        struct.pack_into(format,binary,offset,value)
        glb.binary=bytes(binary)

    def test_actual_three_glb_sources_have_thirty_six_deforming_clips(self):
        for role in ("Hero","Guard","Raider"):
            with self.subTest(role=role):
                report=VALIDATOR.validate(ROOT/"ArtSource/Animated_V78"/("Askir_"+role+".glb"))
                self.assertEqual(report["skin_joints"],18)
                self.assertEqual(set(report["animations"]),VALIDATOR.CLIPS)
                self.assertTrue(all(clip["maximum_sampled_vertex_displacement_m"]>.0001
                                    for clip in report["animations"].values()))

    def test_corrupted_actual_weight_buffer_is_rejected(self):
        glb=self.source()
        index=glb.data["meshes"][0]["primitives"][0]["attributes"]["WEIGHTS_0"]
        self.assertEqual(glb.data["accessors"][index]["componentType"],5126)
        self.overwrite(glb,index,"<f",2.0)
        with patch.object(VALIDATOR,"GLB",return_value=glb), self.assertRaisesRegex(ValueError,"Unnormalized"):
            VALIDATOR.validate("modified binary fixture")

    def test_corrupted_actual_joint_index_is_rejected(self):
        glb=self.source()
        index=glb.data["meshes"][0]["primitives"][0]["attributes"]["JOINTS_0"]
        code=glb.data["accessors"][index]["componentType"]
        self.overwrite(glb,index,"<B" if code==5121 else "<H",99)
        with patch.object(VALIDATOR,"GLB",return_value=glb), self.assertRaisesRegex(ValueError,"joint indices"):
            VALIDATOR.validate("modified binary fixture")

    def test_singular_inverse_bind_buffer_is_rejected(self):
        glb=self.source()
        index=glb.data["skins"][0]["inverseBindMatrices"]
        self.overwrite(glb,index,"<f",0.0)
        with patch.object(VALIDATOR,"GLB",return_value=glb), self.assertRaisesRegex(ValueError,"singular"):
            VALIDATOR.validate("modified binary fixture")

    def test_missing_cast_animation_cannot_pass_asset_acceptance(self):
        glb=self.source()
        glb.data["animations"]=[clip for clip in glb.data["animations"] if clip["name"]!="Cast"]
        with patch.object(VALIDATOR,"GLB",return_value=glb), self.assertRaisesRegex(ValueError,"twelve required"):
            VALIDATOR.validate("missing clip fixture")


if __name__=="__main__":
    unittest.main()
