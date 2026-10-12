import unittest
from Tools.validate_campaign_graph_v72 import audit


def level(name, exits):
    return {'map_name': name, 'continent': 'Ardem', 'chapter': 1, 'exits': exits}


class CampaignGraphAuditTests(unittest.TestCase):
    def test_detects_missing_destination(self):
        result = audit({'levels': [level('L01_Dornfeld', ['Missing'])]})
        self.assertTrue(any('unknown destination' in x for x in result['errors']))

    def test_detects_unreachable(self):
        result = audit({'levels': [level('L01_Dornfeld', []), level('Other', [])]})
        self.assertTrue(any('Unreachable' in x for x in result['errors']))

    def test_detects_duplicate_ids(self):
        result = audit({'levels': [level('L01_Dornfeld', []), level('L01_Dornfeld', [])]})
        self.assertTrue(any('Duplicate' in x for x in result['errors']))

    def test_reports_one_way_as_warning(self):
        result = audit({'levels': [level('L01_Dornfeld', ['Other']), level('Other', [])]})
        self.assertTrue(any('one-way' in x for x in result['warnings']))


if __name__ == '__main__':
    unittest.main()
