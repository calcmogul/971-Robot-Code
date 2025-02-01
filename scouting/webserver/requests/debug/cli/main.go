// This binary lets users interact with the scouting web server in order to
// debug it. Run with `--help` to see all the options.

package main

import (
	"flag"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"

	"github.com/davecgh/go-spew/spew"
	"github.com/frc971/971-Robot-Code/scouting/webserver/requests/debug"
)

// Returns the absolute path of the specified path. This is an unwrapped
// version of `filepath.Abs`.
func absPath(path string) string {
	result, err := filepath.Abs(path)
	if err != nil {
		log.Fatal("Failed to determine absolute path for ", path, ": ", err)
	}
	return result
}

// Parses the specified JSON file into a binary version (i.e. serialized
// flatbuffer). This uses the `flatc` binary and the JSON's corresponding
// `.fbs` file.
func parseJson(fbsPath string, jsonPath string) []byte {
	// Work inside a temporary directory since `flatc` doesn't allow us to
	// customize the name of the output file.
	dir, err := ioutil.TempDir("", "webserver_debug_cli")
	if err != nil {
		log.Fatal("Failed to create temporary directory: ", err)
	}
	defer os.RemoveAll(dir)

	// Turn these paths absolute so that it everything still works from
	// inside the temporary directory.
	absFlatcPath := absPath("external/com_github_google_flatbuffers/flatc")
	absFbsPath := absPath(fbsPath)

	// Create a symlink to the .fbs file so that the output filename that
	// `flatc` generates is predictable. I.e. `fb.json` gets serialized
	// into `fb.bin`.
	jsonSymlink := filepath.Join(dir, "fb.json")
	os.Symlink(jsonPath, jsonSymlink)

	// Execute the `flatc` command.
	flatcCommand := exec.Command(absFlatcPath, "--binary", absFbsPath, jsonSymlink)
	flatcCommand.Dir = dir
	output, err := flatcCommand.CombinedOutput()
	if err != nil {
		log.Fatal("Failed to execute flatc: ", err, ": ", string(output))
	}

	// Read the serialized flatbuffer and return it.
	binaryPath := filepath.Join(dir, "fb.bin")
	binaryFb, err := os.ReadFile(binaryPath)
	if err != nil {
		log.Fatal("Failed to read flatc output ", binaryPath, ": ", err)
	}
	return binaryFb
}

func maybePerformRequest[T interface{}](fbName, fbsPath, requestJsonPath, address string, requester func(string, []byte, string) (*T, error)) {
	if requestJsonPath != "" {
		log.Printf("Sending %s to %s", fbName, address)
		binaryRequest := parseJson(fbsPath, requestJsonPath)
		response, err := requester(address, binaryRequest, "debug_cli")
		if err != nil {
			log.Fatalf("Failed %s: %v", fbName, err)
		}
		spew.Dump(*response)
	}
}

func main() {
	// Parse command line arguments.
	indentPtr := flag.String("indent", " ",
		"The indentation to use for the result dumping. Default is a space.")
	addressPtr := flag.String("address", "http://localhost:8080",
		"The end point where the server is listening.")
	submitDriverRankingPtr := flag.String("submitDriverRanking", "",
		"If specified, parse the file as a submitDriverRanking JSON request.")
	submitDriverRanking2025Ptr := flag.String("submitDriverRanking2025", "",
		"If specified, parse the file as a submitDriverRanking2025 JSON request.")
	submitHumanRanking2025Ptr := flag.String("submitHumanRanking2025", "",
		"If specified, parse the file as a submitHumanRanking2025 JSON request.")
	submitNotesPtr := flag.String("submitNotes", "",
		"If specified, parse the file as a submitNotes JSON request.")
	requestAllMatchesPtr := flag.String("requestAllMatches", "",
		"If specified, parse the file as a RequestAllMatches JSON request.")
	requestAllDriverRankingsPtr := flag.String("requestAllDriverRankings", "",
		"If specified, parse the file as a requestAllDriverRankings JSON request.")
	requestAveragedDriverRankings2025Ptr := flag.String("requestAveragedDriverRankings2025", "",
		"If specified, parse the file as a requestAveragedDriverRankings2025 JSON request.")
	requestAveragedHumanRankings2025Ptr := flag.String("requestAveragedHumanRankings2025", "",
		"If specified, parse the file as a requestAveragedHumanRankings2025 JSON request.")
	requestAllNotesPtr := flag.String("requestAllNotes", "",
		"If specified, parse the file as a requestAllNotes JSON request.")
	requestCurrentScoutingPtr := flag.String("requestCurrentScouting", "",
		"If specified, parse the file as a requestCurrentScouting JSON request.")
	flag.Parse()

	spew.Config.Indent = *indentPtr

	// Disable pointer addresses. They're not useful for our purposes.
	spew.Config.DisablePointerAddresses = true

	// Handle the actual arguments.
	maybePerformRequest(
		"submitNotes",
		"scouting/webserver/requests/messages/submit_notes.fbs",
		*submitNotesPtr,
		*addressPtr,
		debug.SubmitNotes)

	maybePerformRequest(
		"submitDriverRanking",
		"scouting/webserver/requests/messages/submit_driver_ranking.fbs",
		*submitDriverRankingPtr,
		*addressPtr,
		debug.SubmitDriverRanking)
	maybePerformRequest(
		"submitDriverRanking2025",
		"scouting/webserver/requests/messages/submit_driver_ranking_2025.fbs",
		*submitDriverRanking2025Ptr,
		*addressPtr,
		debug.SubmitDriverRanking2025)
	maybePerformRequest(
		"submitHumanRanking2025",
		"scouting/webserver/requests/messages/submit_human_ranking_2025.fbs",
		*submitHumanRanking2025Ptr,
		*addressPtr,
		debug.SubmitHumanRanking2025)

	maybePerformRequest(
		"RequestAllMatches",
		"scouting/webserver/requests/messages/request_all_matches.fbs",
		*requestAllMatchesPtr,
		*addressPtr,
		debug.RequestAllMatches)

	maybePerformRequest(
		"requestAllDriverRankings",
		"scouting/webserver/requests/messages/request_all_driver_rankings.fbs",
		*requestAllDriverRankingsPtr,
		*addressPtr,
		debug.RequestAllDriverRankings)
	maybePerformRequest(
		"requestAveragedDriverRankings2025",
		"scouting/webserver/requests/messages/request_averaged_driver_rankings_2025.fbs",
		*requestAveragedDriverRankings2025Ptr,
		*addressPtr,
		debug.RequestAveragedDriverRankings2025)
	maybePerformRequest(
		"requestAveragedHumanRankings2025",
		"scouting/webserver/requests/messages/request_averaged_human_rankings_2025.fbs",
		*requestAveragedHumanRankings2025Ptr,
		*addressPtr,
		debug.RequestAveragedHumanRankings2025)

	maybePerformRequest(
		"requestAllNotes",
		"scouting/webserver/requests/messages/request_all_notes.fbs",
		*requestAllNotesPtr,
		*addressPtr,
		debug.RequestAllNotes)

	maybePerformRequest(
		"requestCurrentScouting",
		"scouting/webserver/requests/messages/request_current_scouting.fbs",
		*requestCurrentScoutingPtr,
		*addressPtr,
		debug.RequestCurrentScouting)
}
