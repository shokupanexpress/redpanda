// Copyright 2020 Redpanda Data, Inc.
//
// Use of this software is governed by the Business Source License
// included in the file licenses/BSL.md
//
// As of the Change Date specified in that file, in accordance with
// the Business Source License, use of this software will be governed
// by the Apache License, Version 2.0

package filesystem

import "errors"

func GetFilesystemType(path string) (FsType, error) {
	return Unknown, errors.New("Filesystem detection not available for MacOS")
}
