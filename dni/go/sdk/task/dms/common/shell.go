package common

import (
	"fmt"
	"log"
	"os/exec"
	"strings"
)

// Run bash to execute cmd. Return combined output (stdout+stderr) and error if exists
func RunShellCmd(cmd string) (string, error) {
	shellCmd := exec.Command("/bin/bash", "-c", cmd)
	out, err := shellCmd.CombinedOutput()
	if err != nil {
		return "", err
	}
	return string(out), nil
}
func RunShellCmdf(cmdt string, args ...interface{}) (string, error) {
	return RunShellCmd(fmt.Sprintf(cmdt, args...))
}

// Run bash to execute cmd. Return output (stdout) and error if exists
func CmdOutput(cmd string) (string, error) {
	shellCmd := exec.Command("/bin/bash", "-c", cmd)
	out, err := shellCmd.Output()
	if err != nil {
		return "", err
	}
	return string(out), nil
}
func CmdOutputf(cmdt string, args ...interface{}) (string, error) {
	return CmdOutput(fmt.Sprintf(cmdt, args...))
}

// Join cmd output into one line
func CmdOutputOneline(cmd string) string {
	str, err := RunShellCmd(cmd)
	if err != nil {
		return ""
	}
	str = strings.ReplaceAll(str, "\n", "")
	str = strings.Trim(str, " ")
	return str
}
func CmdOutputOnelinef(cmdt string, args ...interface{}) string {
	return CmdOutputOneline(fmt.Sprintf(cmdt, args...))
}

// Split cmd output into lines
func CmdOutputLines(cmd string) []string {
	str, err := RunShellCmd(cmd)
	if err != nil {
		return nil
	}
	return strings.Split(strings.TrimRight(str, "\n"), "\n")
}
func CmdOutputLinesf(cmdt string, args ...interface{}) []string {
	return CmdOutputLines(fmt.Sprintf(cmdt, args...))
}

// Split cmd output into fields
func CmdOutputFields(cmd string) []string {
	str, err := RunShellCmd(cmd)
	if err != nil {
		return nil
	}
	return strings.Fields(str)
}
func CmdOutputFieldsf(cmdt string, args ...interface{}) []string {
	return CmdOutputFields(fmt.Sprintf(cmdt, args...))
}

func Grep(pattern, fpath string) bool {
	cmd := exec.Command("grep", "-w", pattern, fpath)
	err := cmd.Run()
	switch r := cmd.ProcessState.ExitCode(); r {
	case 0:
		return true
	case 1: // No error with grep, but no pattern found in file
		return false
	default:
		log.Printf("failed to execute grep %q for %q, err: %v", pattern, fpath, err)
		return false
	}
}

func Pgrep(process string) bool {
	cmd := exec.Command("pgrep", process)
	err := cmd.Run()
	switch r := cmd.ProcessState.ExitCode(); r {
	case 0:
		return true
	case 1:
		return false
	default:
		log.Printf("failed to execute pgrep %q, err: %v", process, err)
		return false
	}
}
