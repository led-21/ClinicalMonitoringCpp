#include "IcuBedManagementTests.h"

#include "clinical/ICU.h"

using namespace clinical;

namespace
{
    void ShouldCreatePatientAtValidBed()
    {
        ICU icu;
        auto* patient = icu.CreatePatientAtBed(1, "001", "Alice", "Smith");

        AssertTrue(patient != nullptr, "ICU should create patient at bed 1");
        AssertEqual(std::string("001"), patient->GetId(), "Created patient id mismatch");
        AssertEqual(std::string("Alice"), patient->GetFirstName(), "Created patient first name mismatch");
        AssertEqual(std::string("Smith"), patient->GetLastName(), "Created patient last name mismatch");
    }

    void ShouldReturnNullForInvalidBedOnCreate()
    {
        ICU icu;
        AssertTrue(icu.CreatePatientAtBed(0, "X", "A", "B") == nullptr, "Bed 0 should be invalid");
        AssertTrue(icu.CreatePatientAtBed(7, "X", "A", "B") == nullptr, "Bed 7 should be invalid");
        AssertTrue(icu.CreatePatientAtBed(-1, "X", "A", "B") == nullptr, "Negative bed index should be invalid");
    }

    void ShouldNotOverwriteOccupiedBed()
    {
        ICU icu;
        auto* first = icu.CreatePatientAtBed(3, "111", "Bob", "Jones");
        auto* second = icu.CreatePatientAtBed(3, "222", "Carol", "Lee");

        AssertTrue(first != nullptr, "First patient should be created");
        AssertTrue(second != nullptr, "Create on occupied bed should return existing patient");
        AssertEqual(first, second, "Occupied bed must return the same patient pointer");
        AssertEqual(std::string("111"), second->GetId(), "Occupied bed id must remain unchanged");
    }

    void ShouldGetPatientAtBed()
    {
        ICU icu;
        AssertTrue(icu.GetPatientAtBed(2) == nullptr, "Empty bed should return nullptr");

        icu.CreatePatientAtBed(2, "042", "Dan", "Brown");
        auto* found = icu.GetPatientAtBed(2);
        AssertTrue(found != nullptr, "Should retrieve patient at bed 2");
        AssertEqual(std::string("042"), found->GetId(), "Retrieved patient id mismatch");
    }

    void ShouldReturnNullForInvalidBedOnGet()
    {
        ICU icu;
        AssertTrue(icu.GetPatientAtBed(0) == nullptr, "GetPatientAtBed(0) should be nullptr");
        AssertTrue(icu.GetPatientAtBed(7) == nullptr, "GetPatientAtBed(7) should be nullptr");
    }

    void ShouldFindPatientByIdAndReportBed()
    {
        ICU icu;
        icu.CreatePatientAtBed(5, "999", "Eve", "Taylor");

        int bedNumber = 0;
        auto* found = icu.FindPatientById("999", &bedNumber);

        AssertTrue(found != nullptr, "Should find patient by id");
        AssertEqual(5, bedNumber, "Should report correct bed number for found patient");
    }

    void ShouldReturnNullWhenPatientNotFound()
    {
        ICU icu;
        AssertTrue(icu.FindPatientById("does-not-exist") == nullptr, "Should return nullptr for unknown id");
    }

    void ShouldListOccupiedBeds()
    {
        ICU icu;
        AssertTrue(icu.GetOccupiedBeds().empty(), "Empty ICU should have no occupied beds");

        icu.CreatePatientAtBed(1, "A1", "F", "G");
        icu.CreatePatientAtBed(6, "A2", "H", "I");

        const auto occupied = icu.GetOccupiedBeds();
        AssertEqual(2, static_cast<int>(occupied.size()), "ICU should report 2 occupied beds");
        AssertEqual(1, occupied[0].first, "First occupied bed should be 1");
        AssertEqual(6, occupied[1].first, "Second occupied bed should be 6");
    }

    void ShouldFillAllSixBeds()
    {
        ICU icu;
        for (int i = 1; i <= 6; ++i)
        {
            AssertTrue(
                icu.CreatePatientAtBed(i, std::to_string(i), "P", "Q") != nullptr,
                "Should create patient at bed " + std::to_string(i));
        }
        AssertEqual(6, static_cast<int>(icu.GetOccupiedBeds().size()), "ICU should have all 6 beds occupied");
    }

    void ShouldCreateFindAndDischargePatient()
    {
        ICU icu;
        auto* patient = icu.CreatePatientAtBed(2, "251", "Jane", "Doe");

        AssertTrue(patient != nullptr, "Should create patient in requested bed");
        AssertEqual(std::string("251"), patient->GetId(), "Created patient id mismatch");

        int bedNumber = 0;
        auto* found = icu.FindPatientById("251", &bedNumber);
        AssertTrue(found != nullptr, "Should find patient by id");
        AssertEqual(2, bedNumber, "Should report correct bed number");

        std::string fullName;
        const bool discharged = icu.DischargePatientById("251", &fullName);
        AssertTrue(discharged, "Should discharge an existing patient");
        AssertEqual(std::string("Jane Doe"), fullName, "Discharged patient name mismatch");
        AssertTrue(icu.FindPatientById("251") == nullptr, "Discharged patient should be removed from ICU");
    }

    void ShouldReturnFalseWhenDischargingUnknownPatient()
    {
        ICU icu;
        const bool discharged = icu.DischargePatientById("999");
        AssertTrue(!discharged, "Should return false when patient id is unknown");
    }

    void ShouldFreeBedAfterDischarge()
    {
        ICU icu;
        icu.CreatePatientAtBed(4, "AAA", "Tom", "Lee");
        icu.DischargePatientById("AAA");

        // Bed 4 should be free and reusable
        auto* newPatient = icu.CreatePatientAtBed(4, "BBB", "Sara", "Kim");
        AssertTrue(newPatient != nullptr, "Discharged bed should be reusable");
        AssertEqual(std::string("BBB"), newPatient->GetId(), "New patient at reused bed id mismatch");
    }

    void ShouldDischargeWithoutFullNameParam()
    {
        ICU icu;
        icu.CreatePatientAtBed(1, "XYZ", "No", "Name");
        const bool discharged = icu.DischargePatientById("XYZ", nullptr);
        AssertTrue(discharged, "Discharge with null name param should succeed");
    }

    void ShouldGetOccupiedBedsWithMultiplePatients()
    {
        ICU icu;
        icu.CreatePatientAtBed(1, "P1", "A", "B");
        icu.CreatePatientAtBed(3, "P2", "C", "D");
        icu.CreatePatientAtBed(6, "P3", "E", "F");

        const auto occupied = icu.GetOccupiedBeds();
        AssertEqual(3, static_cast<int>(occupied.size()), "Should report 3 occupied beds");
        AssertEqual(1, occupied[0].first, "First occupied bed should be 1");
        AssertEqual(3, occupied[1].first, "Second occupied bed should be 3");
        AssertEqual(6, occupied[2].first, "Third occupied bed should be 6");
    }

    void ShouldReportEmptyBedsAfterAllDischarged()
    {
        ICU icu;
        icu.CreatePatientAtBed(2, "D1", "A", "B");
        icu.CreatePatientAtBed(5, "D2", "C", "D");
        icu.DischargePatientById("D1");
        icu.DischargePatientById("D2");

        AssertTrue(icu.GetOccupiedBeds().empty(), "All discharged ICU should report no occupied beds");
    }

    void ShouldFindPatientWithoutBedNumberParam()
    {
        ICU icu;
        icu.CreatePatientAtBed(3, "Q1", "X", "Y");
        auto* found = icu.FindPatientById("Q1", nullptr);
        AssertTrue(found != nullptr, "FindPatientById with null bed param should work");
    }

    void ShouldUseModernFindAndDischargeOptionalApi()
    {
        ICU icu;
        icu.CreatePatientAtBed(3, "P042", "Arthur", "Dent");

        // Modern FindPatient returning std::optional<BedLookupResult> with structured binding
        const auto lookup = icu.FindPatient("P042");
        AssertTrue(lookup.has_value(), "Modern FindPatient should find patient");
        AssertEqual(3, lookup->bedNumber, "Bed number should match");
        AssertEqual(std::string("P042"), lookup->patient->GetId(), "Patient id should match");

        // Const overload
        const ICU& constIcu = icu;
        const auto constLookup = constIcu.FindPatient("P042");
        AssertTrue(constLookup.has_value(), "Const FindPatient should find patient");
        AssertEqual(3, constLookup->bedNumber, "Const lookup bed number should match");

        // Modern DischargePatient returning std::optional<std::string>
        const auto dischargedName = icu.DischargePatient("P042");
        AssertTrue(dischargedName.has_value(), "Discharge should succeed");
        AssertEqual(std::string("Arthur Dent"), dischargedName.value(), "Discharged full name should match");
        AssertTrue(!icu.FindPatient("P042").has_value(), "Patient should no longer exist after discharge");
    }
}

void RegisterIcuBedManagementTests(std::vector<TestCase>& tests)
{
    tests.push_back({ "ICU - Create patient at valid bed",             ShouldCreatePatientAtValidBed });
    tests.push_back({ "ICU - Reject invalid bed numbers on create",    ShouldReturnNullForInvalidBedOnCreate });
    tests.push_back({ "ICU - Do not overwrite occupied bed",           ShouldNotOverwriteOccupiedBed });
    tests.push_back({ "ICU - Get patient at bed",                      ShouldGetPatientAtBed });
    tests.push_back({ "ICU - Return null for invalid bed on get",      ShouldReturnNullForInvalidBedOnGet });
    tests.push_back({ "ICU - Find patient by id with bed number",      ShouldFindPatientByIdAndReportBed });
    tests.push_back({ "ICU - Return null for unknown patient id",      ShouldReturnNullWhenPatientNotFound });
    tests.push_back({ "ICU - List occupied beds",                      ShouldListOccupiedBeds });
    tests.push_back({ "ICU - Fill all six beds",                       ShouldFillAllSixBeds });
    tests.push_back({ "ICU - Create, find and discharge patient",      ShouldCreateFindAndDischargePatient });
    tests.push_back({ "ICU - Fail discharge for unknown patient",      ShouldReturnFalseWhenDischargingUnknownPatient });
    tests.push_back({ "ICU - Bed is free and reusable after discharge", ShouldFreeBedAfterDischarge });
    tests.push_back({ "ICU - Discharge with null name param",          ShouldDischargeWithoutFullNameParam });
    tests.push_back({ "ICU - List multiple occupied beds",             ShouldGetOccupiedBedsWithMultiplePatients });
    tests.push_back({ "ICU - Empty beds after all discharged",         ShouldReportEmptyBedsAfterAllDischarged });
    tests.push_back({ "ICU - Find patient without bed param",          ShouldFindPatientWithoutBedNumberParam });
    tests.push_back({ "ICU - Modern std::optional Find and Discharge API", ShouldUseModernFindAndDischargeOptionalApi });
}
