"""Offline encounter state machine, independent of Unreal. Python 3.10+."""
from dataclasses import dataclass
from enum import Enum
class Phase(str,Enum):
    DORMANT='dormant'; ACTIVE='active'; CLEARED='cleared'
@dataclass
class Encounter:
    encounter_id:str
    enemy_count:int
    phase:Phase=Phase.DORMANT
    remaining:int=0
    def activate(self, distance_cm:float, radius_cm:float, gate_open:bool=True)->bool:
        if self.phase!=Phase.DORMANT or not gate_open or distance_cm>radius_cm:return False
        self.phase=Phase.ACTIVE;self.remaining=self.enemy_count;return True
    def enemy_defeated(self)->bool:
        if self.phase!=Phase.ACTIVE or self.remaining<=0:return False
        self.remaining-=1
        if self.remaining==0:self.phase=Phase.CLEARED
        return True
    def reset_on_map_reload(self):
        self.phase=Phase.DORMANT;self.remaining=0
    def snapshot(self):return {'id':self.encounter_id,'phase':self.phase.value,'remaining':self.remaining}
    def restore(self,snapshot):
        if snapshot['id']!=self.encounter_id:raise ValueError('encounter id mismatch')
        phase=Phase(snapshot['phase']);rem=snapshot['remaining']
        if not isinstance(rem,int) or isinstance(rem,bool) or rem<0 or rem>self.enemy_count:raise ValueError('invalid remaining')
        if (phase==Phase.DORMANT and rem!=0) or (phase==Phase.CLEARED and rem!=0) or (phase==Phase.ACTIVE and rem==0):raise ValueError('inconsistent state')
        self.phase=phase;self.remaining=rem
